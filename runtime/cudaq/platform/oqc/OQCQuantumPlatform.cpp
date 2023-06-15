#include "cudaq/platform/qpu.h"
#include "cudaq/platform/quantum_platform.h"
#include "cudaq/platform/default/rest/Executor.h"
// #include "cudaq/platform/default/rest/RemoteRestQPU.cpp"
#include "common/ServerHelper.h"
#include "common/NoiseModel.h"
#include "common/ExecutionContext.h"
#include "common/Logger.h"
#include "cudaq/platform/default/rest/helpers/oqc/OQCServerHelp.cpp"
#include <stdexcept>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <common/Future.h>

using cudaq::QPU;
using cudaq::noise_model;
using cudaq::QuantumTask;
using cudaq::ExecutionContext;
using namespace cudaq;

namespace {

class JamieExecutor{
protected:
  /// @brief The ServerHelper, providing system-specific JSON-formatted
  /// job posts and results translation

  /// @brief The number of shots to execute
  std::size_t shots = 100;

public:
  std::unique_ptr<OQCServerHelper> serverHelper;

  JamieExecutor() = default;
  virtual ~JamieExecutor() = default;

  /// @brief Set the server helper
  void setServerHelper(std::unique_ptr<OQCServerHelper> helper) { 
    serverHelper = std::move(helper); 
    }

  /// @brief Set the number of shots to execute
  void setShots(std::size_t s) { shots = s; }

  /// @brief Execute the provided quantum codes and return a future object
  /// The caller can make this synchronous by just immediately calling .get().

  details::future execute(std::vector<KernelExecution> &codesToExecute) {
    shots = 1000;
    serverHelper->setShots(shots);


    // cudaq::info("Executor creating {} jobs to execute with the {} helper.",
                // codesToExecute.size(), serverHelper->name());

    // Create the Job Payload, composed of job post path, headers,
    // and the job json messages themselves
    std::cout<<"creating job\n";
    auto [jobPostPath, headers, jobs] = serverHelper->createJob(codesToExecute);
    std::cout<<"created job\n";


    std::vector<details::future::Job> ids;
    for (std::size_t i = 0; auto &job : jobs) {
      // cudaq::info("Job (name={}) created, posting to {}", codesToExecute[i].name,
                  // jobPostPath);

      // Post it, get the response
      auto response = serverHelper->client.post("http://localhost:5000", jobPostPath, job, headers);
      // cudaq::info("Job (name={}) posted, response was {}", codesToExecute[i].name,
                  // response.dump());
      std::cout<<response<<"\n";

      // Add the job id and the job name.
      // std::cout<<job<<"\n";
      // std::cout<<job.at("tasks")<<"\n";
      std::cout<<job.at("tasks")[0].at("task_id")<<"\n";
      ids.emplace_back(std::pair(job.at("tasks")[0].at("task_id"), codesToExecute[i].name));
      i++;
    }
    // for (std::size_t i = 0; auto &job : jobs) {
    //   std::string task_id = job.at("tasks")[0].at("task_id");
    //   std::string url = "/tasks/"+task_id+"/results";
    //   auto results = serverHelper->getResults(url);
    //   std::cout<< results << "\n";
    // }

    auto config = serverHelper->getConfig();
    std::string name = serverHelper->name();
    return details::future(ids, name, config);
  }
};


class OQCQPU : public QPU {
protected:
  std::unique_ptr<JamieExecutor> executor;

  std::map<std::string, std::string> backendConfig;

public:
  OQCQPU(std::size_t _qpuId = 0, std::size_t nQubits = 8) : QPU() {
    numQubits = nQubits;
    executor = std::make_unique<JamieExecutor>();
  };

  virtual ~OQCQPU() = default;

  virtual bool isSimulator() override { 
    
    return false; 
  }

  /// Enqueue a quantum task on the asynchronous execution queue.
  virtual void enqueue(QuantumTask &task) override {

    // TODO AFAIK this is where we can step in and get these asysnc
    // promises executed via QAT QIR frontend
    execution_queue->enqueue(task);
  }

  /// Set the execution context, meant for subtype specification
  void setExecutionContext(cudaq::ExecutionContext *context) override {
    if (!context)
      return;

    cudaq::info("Remote Rest QPU setting execution context to {}",
                context->name);

    // Execution context is valid
    executionContext = context;
  }
  virtual void resetExecutionContext() override {
    executionContext = nullptr;
  }

  void setTargetBackend(const std::string &backend) override{
    cudaq::info("Remote REST platform is targeting {}.", backend);
    std::cout<< "setting backend\n";
    auto oqc_server = std::make_unique<cudaq::OQCServerHelper>();
    std::cout<< "do I make it after the oqc server init?\n";
    oqc_server->initialize(backendConfig);

    executor->setServerHelper(std::move(oqc_server));
    std::cout<< "do I make it after the helper?\n";

  }

  void launchKernel(const std::string &kernelName, void (*kernelFunc)(void *),
                    void *args, std::uint64_t voidStarSize,
                    std::uint64_t resultOffset) override {
    cudaq::info("launching remote rest kernel ({})", kernelName);

    // TODO future iterations of this should support non-void return types.
    if (!executionContext)
      throw std::runtime_error("Remote rest execution can only be performed "
                               "via cudaq::sample() or cudaq::observe().");

    std::string name = "blarg";
    std::string code = "OPENQASM 2.0;include \"qelib1.inc\";qreg q[2];creg c[2];h q[0]; barrier q;measure q[0]->c[1];measure q[1]->c[1];";
    std::vector<cudaq::KernelExecution> codes{
      cudaq::KernelExecution(
        name,
        code
      ),
    };
    std::cout<<"we're excuting, woohoo\n";

    // Execute the codes produced in quake lowering
    auto future = executor->execute(codes);
    executionContext->result = future.get();
    cudaq::ExecutionResult result = cudaq::ExecutionResult(
          double(0.32)
      );
    executionContext->result=cudaq::sample_result(result);
  }
};

class OQCQuantumPlatform : public cudaq::quantum_platform {
public:
  OQCQuantumPlatform() {
    // Populate the information and add the QPUs
    platformQPUs.emplace_back(std::make_unique<OQCQPU>());
    platformNumQPUs = platformQPUs.size();
  }
  virtual void setTargetBackend(const std::string &name) override {
    std::cout<< "setting backend initial\n";
    platformQPUs[0]->setTargetBackend(name);
  }
};

}

CUDAQ_REGISTER_PLATFORM(OQCQuantumPlatform, oqc)
