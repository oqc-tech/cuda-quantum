#include "cudaq/platform/qpu.h"
#include "cudaq/platform/quantum_platform.h"
// #include "cudaq/platform/default/rest/Executor.h"
#include "cudaq/platform/default/rest/RemoteRESTQPU.h"
#include "common/NoiseModel.h"
#include "common/ExecutionContext.h"
#include "common/Logger.h"
#include "cudaq/platform/default/rest/helpers/oqc/OQCServerHelp.cpp"
#include <stdexcept>
#include <memory>
#include <iostream>

using cudaq::QPU;
using cudaq::noise_model;
using cudaq::QuantumTask;
using cudaq::ExecutionContext;

namespace {
class OQCQPU : public RemoteRESTQPU {
protected:
  std::unique_ptr<cudaq::Executor> executor;

  std::map<std::string, std::string> backendConfig;

public:
  OQCQPU(std::size_t _qpuId = 0, std::size_t nQubits = 8) : RemoteRESTQPU() {
    numQubits = nQubits;
    executor = std::make_unique<cudaq::Executor>();
  };

  virtual ~OQCQPU() = default;

  virtual bool isSimulator() override { 
    std::cout << "AM I A SMIULATOR? \n";
    
    return false; 
  }

  /// Enqueue a quantum task on the asynchronous execution queue.
  virtual void enqueue(QuantumTask &task) override {

    // TODO AFAIK this is where we can step in and get these asysnc
    // promises executed via QAT QIR frontend
    std::cout << "\n TEST TEST TEST enque \n";
    execution_queue->enqueue(task);
  }

  /// Set the execution context, meant for subtype specification
  virtual void setExecutionContext(ExecutionContext *context) override {
    cudaq::ScopedTrace trace("OQCPlatform::setExecutionContext", context->name);
    executionContext = context;
    if (noiseModel)
      executionContext->noiseModel = noiseModel;

    cudaq::getExecutionManager()->setExecutionContext(executionContext);
  }
  virtual void resetExecutionContext() override {
    executionContext = nullptr;
  }

  void setTargetBackend(const std::string &backend) override{
    cudaq::info("Remote REST platform is targeting {}.", backend);
    std::cout<< "setting backend\n";
    cudaq::OQCServerHelper oqc_server = cudaq::OQCServerHelper();
    std::cout<< "do I make it after the oqc server init?\n";
    oqc_server.initialize(backendConfig);

    executor->setServerHelper(&oqc_server);
    std::cout<< "do I make it after the helper?\n";

  }
  // Pointer to the concrete Executor for this QPU
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
