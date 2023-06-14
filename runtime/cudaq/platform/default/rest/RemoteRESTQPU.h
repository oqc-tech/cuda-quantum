/*************************************************************** -*- C++ -*- ***
 * Copyright (c) 2022 - 2023 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 *******************************************************************************/

#include "Executor.h"
#include "common/ExecutionContext.h"
#include "common/Logger.h"
#include "common/RestClient.h"
#include "cudaq/platform/qpu.h"
#include "nvqpp_config.h"

#include "common/FmtCore.h"
#include "common/RuntimeMLIR.h"
#include "cudaq/platform/quantum_platform.h"
#include <cudaq/spin_op.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <regex>
#include <sys/socket.h>
#include <sys/types.h>

// #include "cudaq/Frontend/nvqpp/AttributeNames.h"
// #include "cudaq/Optimizer/CodeGen/Passes.h"
// #include "cudaq/Optimizer/Dialect/CC/CCDialect.h"
// #include "cudaq/Optimizer/Dialect/Quake/QuakeDialect.h"
// #include "cudaq/Optimizer/Transforms/Passes.h"
// #include "cudaq/Support/Plugin.h"
// #include "cudaq/Target/OpenQASM/OpenQASMEmitter.h"
// #include "llvm/Bitcode/BitcodeWriter.h"
// #include "llvm/IR/Module.h"
// #include "llvm/Support/Base64.h"
// #include "mlir/Dialect/Affine/IR/AffineOps.h"
// #include "mlir/Dialect/Arith/IR/Arith.h"
// #include "mlir/Dialect/Func/IR/FuncOps.h"
// #include "mlir/Dialect/LLVMIR/LLVMDialect.h"
// #include "mlir/Dialect/Math/IR/Math.h"
// #include "mlir/Dialect/MemRef/IR/MemRef.h"
// #include "mlir/ExecutionEngine/OptUtils.h"
// #include "mlir/IR/BuiltinOps.h"
// #include "mlir/IR/Diagnostics.h"
// #include "mlir/IR/ImplicitLocOpBuilder.h"
// #include "mlir/Parser/Parser.h"
// #include "mlir/Pass/PassManager.h"
// #include "mlir/Pass/PassRegistry.h"
// #include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
// #include "mlir/Target/LLVMIR/Export.h"
// #include "mlir/Tools/mlir-translate/Translation.h"

using namespace mlir;

namespace cudaq {
std::string get_quake_by_name(const std::string &);
} // namespace cudaq

namespace {

constexpr char platformLoweringConfig[] = "PLATFORM_LOWERING_CONFIG";
constexpr char codeEmissionType[] = "CODEGEN_EMISSION";

/// @brief The RemoteRESTQPU is a subtype of QPU that enables the
/// execution of CUDA Quantum kernels on remotely hosted quantum computing
/// services via a REST Client / Server interaction. This type is meant
/// to be general enough to support any remotely hosted service. Specific
/// details about JSON payloads are abstracted via an abstract type called
/// ServerHelper, which is meant to be subtyped by each provided remote QPU
/// service. Moreover, this QPU handles launching kernels under a number of
/// Execution Contexts, including sampling and observation via synchronous or
/// asynchronous client invocations. This type should enable both QIR-based
/// backends as well as those that take OpenQASM2 as input.
class RemoteRESTQPU : public cudaq::QPU {
protected:
  /// The number of shots
  std::optional<int> nShots;

  /// @brief the platform file path, CUDAQ_INSTALL/platforms
  std::filesystem::path platformPath;

  /// @brief The Pass pipeline string, configured by the
  /// QPU config file in the platform path.
  std::string passPipelineConfig;

  /// @brief The name of the QPU being targeted
  std::string qpuName;

  std::string codegenTranslation;

  // Pointer to the concrete Executor for this QPU
  std::unique_ptr<cudaq::Executor> executor;

  /// @brief Pointer to the concrete ServerHelper, provides
  /// specific JSON payloads and POST/GET URL paths.
  std::unique_ptr<cudaq::ServerHelper> serverHelper;

  /// @brief Mapping of general key-values for backend
  /// configuration.
  std::map<std::string, std::string> backendConfig;

public:
  /// @brief The constructor
  RemoteRESTQPU() : QPU() {
    std::filesystem::path cudaqLibPath{cudaq::getCUDAQLibraryPath()};
    platformPath = cudaqLibPath.parent_path().parent_path() / "platforms";
    // Default is to run sampling via the remote rest call
    executor = std::make_unique<cudaq::Executor>();
  }

  RemoteRESTQPU(RemoteRESTQPU &&) = delete;
  virtual ~RemoteRESTQPU() = default;

//   void enqueue(cudaq::QuantumTask &task) override;

  /// @brief Return true if the current backend is a simulator
  /// @return
//   bool isSimulator() override;

  /// @brief Return true if the current backend supports conditional feedback
//   bool supportsConditionalFeedback() override;

  /// Provide the number of shots
//   void setShots(int _nShots) override;

  /// Clear the number of shots
//   void clearShots() override;
//   virtual bool isRemote() override;

  /// Store the execution context for launchKernel
//   void setExecutionContext(cudaq::ExecutionContext *context) override;

  /// Reset the execution context
//   void resetExecutionContext() override;

  /// @brief This setTargetBackend override is in charge of reading the
  /// specific target backend configuration file (bundled as part of this
  /// CUDA Quantum installation) and extract MLIR lowering pipelines and
  /// specific code generation output required by this backend (QIR/QASM2).
//   void setTargetBackend(const std::string &backend) override;

  /// @brief Extract the Quake representation for the given kernel name and
  /// lower it to the code format required for the specific backend. The
  /// lowering process is controllable via the platforms/BACKEND.config file for
  /// this targeted backend.
  std::vector<cudaq::KernelExecution>
  lowerQuakeCode(const std::string &kernelName, void *kernelArgs);

  /// @brief Launch the kernel. Extract the Quake code and lower to
  /// the representation required by the targeted backend. Handle all pertinent
  /// modifications for the execution context as well as async or sync
  /// invocation.
//   void launchKernel(const std::string &kernelName, void (*kernelFunc)(void *),
//                     void *args, std::uint64_t voidStarSize,
//                     std::uint64_t resultOffset) override;
// };
} // namespace

CUDAQ_REGISTER_TYPE(cudaq::QPU, RemoteRESTQPU, remote_rest)
