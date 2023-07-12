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


int main() {

  auto &platform = cudaq::get_platform();
  
  std::string backend = "oqc;emulate;false;url;http://localhost:5000;email;{};password;{}";
  platform.setTargetBackend(backend);
  // platform.is_simulator();

  std::cout<<"creating kernel\n";
  auto kernel = cudaq::make_kernel();
  auto qubit = kernel.qalloc(2);
  kernel.h(qubit[0]);
  kernel.x<cudaq::ctrl>(qubit[0], qubit[1]);
  kernel.mz(qubit[0]);

  std::cout<<"Executing\n";
  auto counts = cudaq::sample(kernel);
  counts.dump();
  
}