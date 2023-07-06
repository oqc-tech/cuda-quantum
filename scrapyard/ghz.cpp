/*************************************************************** -*- C++ -*- ***
 * Copyright (c) 2022 - 2023 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 *******************************************************************************/

/*************************************************************** -*- C++ -*- ***
 * Copyright (c) 2022 - 2023 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 *******************************************************************************/

#include <cudaq.h>
#include <iostream>


struct ghz {
  void operator()(int N) __qpu__ {
    cudaq::qreg q(N);
    h(q[0]);
    for (int i = 0; i < N - 1; i++) {
        cx(q[i], q[i+1]);
    }
    mz(q);
  }
};


int main() {
  // auto &platform = cudaq::get_platform();
  // platform.setTargetBackend("OQC;qpu;url;http://localhost:500;qpu:{};email:j@j.com;password:j;");
  // std::cout << platform.get_current_qpu() << "\n";
  // std::cout << platform.is_remote() << "\n";
  // std::cout << platform.is_simulator() << "\n";

  // auto counts = cudaq::sample(ghz{}, 3);
  // counts.dump();
  // return 0;

  auto &platform = cudaq::get_platform();
  
  std::string backend = "oqc;qpu;/workspaces/cuda-quantum-oqc/runtime/cudaq/platform/default/rest/helpers/oqc/oqc.config";
  platform.setTargetBackend(backend);
  platform.is_simulator();
  auto kernel = cudaq::make_kernel();
  auto qubit = kernel.qalloc(2);
  kernel.h(qubit[0]);
  kernel.mz(qubit[0]);

  auto counts = cudaq::sample(kernel);
  counts.dump();


  
}