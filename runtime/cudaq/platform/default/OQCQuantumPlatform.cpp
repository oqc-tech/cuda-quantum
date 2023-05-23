#include "cudaq/platform/qpu.h"
#include "cudaq/platform/quantum_platform.h"
#include "common/NoiseModel.h"
#include "common/ExecutionContext.h"
#include "common/Logger.h"
#include <stdexcept>
#include <memory>

using cudaq::QPU;
using cudaq::noise_model;
using cudaq::QuantumTask;
using cudaq::ExecutionContext;

namespace {
class OQCQPU : public QPU {
public:
  OQCQPU(std::size_t _qpuId = 0, std::size_t nQubits = 8) : QPU(_qpuId) {
    numQubits = nQubits;
  };

  virtual ~OQCQPU() = default;

  void launchKernel(const std::string &name, void (*kernelFunc)(void *),
                    void *args, std::uint64_t, std::uint64_t) override {
    cudaq::ScopedTrace trace("QPU::launchKernel");
    kernelFunc(args);
  }

  virtual bool isSimulator() override { return false; }

  /// Enqueue a quantum task on the asynchronous execution queue.
  virtual void enqueue(QuantumTask &task) override {
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
    // TODO
    throw std::runtime_error("resetExecutionContext "
                             "not implemented for QQCQPU");
  }
};

class OQCQuantumPlatform : public cudaq::quantum_platform {
public:
  OQCQuantumPlatform() {
    // Populate the information and add the QPUs
    platformQPUs.emplace_back(std::make_unique<OQCQPU>());
    platformNumQPUs = platformQPUs.size();
  }
};

}

//CUDAQ_REGISTER_PLATFORM(OQCQuantumPlatform, oqc)
