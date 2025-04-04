/*******************************************************************************
 * Copyright (c) 2022 - 2025 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

// Compile and run with:
// ```
// nvq++ compute_actions.cpp -o gs.x && ./gs.x
// ```

// The algorithm leverages the VQE support to compute ground
// state of the Hydrogen atom.

#include <cudaq.h>
#include <cudaq/algorithm.h>

#include <map>
#include <vector>

struct ansatz_handcoded {
  void operator()(double theta) __qpu__ {
    cudaq::qvector q(4);
    x(q[0]);
    x(q[2]);
    rx(M_PI_2, q[0]);
    h(q[1]);
    h(q[2]);
    h(q[3]);
    x<cudaq::ctrl>(q[0], q[1]);
    x<cudaq::ctrl>(q[1], q[2]);
    x<cudaq::ctrl>(q[2], q[3]);
    rz(theta, q[3]);
    x<cudaq::ctrl>(q[2], q[3]);
    x<cudaq::ctrl>(q[1], q[2]);
    x<cudaq::ctrl>(q[0], q[1]);
    h(q[3]);
    h(q[2]);
    h(q[1]);
    rx(-M_PI_2, q[0]);
  }
};

// The kernel defined by `ansatz_compute_action` is
// equivalent to the one defined in `ansatz_handcoded`.
struct ansatz_compute_action {
  void operator()(std::vector<double> theta) __qpu__ {
    cudaq::qvector q(4);
    x(q[0]);
    x(q[2]);

    cudaq::compute_action(
        [&]() {
          rx(M_PI_2, q[0]);
          h(q[1]);
          h(q[2]);
          h(q[3]);
          x<cudaq::ctrl>(q[0], q[1]);
          x<cudaq::ctrl>(q[1], q[2]);
          x<cudaq::ctrl>(q[2], q[3]);
        },
        [&]() { rz(theta[0], q[3]); });
  }
};

int main(int argc, char **argv) {

  std::vector<double> h2_data{
      15, -0.10647701149499994, 0, 4, 0, 0, 1, 0, 2, 0, 3,
      0,  0.0454063328691,      0, 4, 0, 2, 1, 2, 2, 2, 3,
      2,  0.0454063328691,      0, 4, 0, 2, 1, 2, 2, 3, 3,
      3,  0.0454063328691,      0, 4, 0, 3, 1, 3, 2, 2, 3,
      2,  0.0454063328691,      0, 4, 0, 3, 1, 3, 2, 3, 3,
      3,  0.170280101353,       0, 4, 0, 1, 1, 0, 2, 0, 3,
      0,  0.120200490713,       0, 4, 0, 1, 1, 1, 2, 0, 3,
      0,  0.168335986252,       0, 4, 0, 1, 1, 0, 2, 1, 3,
      0,  0.165606823582,       0, 4, 0, 1, 1, 0, 2, 0, 3,
      1,  -0.22004130022499996, 0, 4, 0, 0, 1, 1, 2, 0, 3,
      0,  0.165606823582,       0, 4, 0, 0, 1, 1, 2, 1, 3,
      0,  0.174072892497,       0, 4, 0, 0, 1, 1, 2, 0, 3,
      1,  0.170280101353,       0, 4, 0, 0, 1, 0, 2, 1, 3,
      0,  0.120200490713,       0, 4, 0, 0, 1, 0, 2, 1, 3,
      1,  -0.22004130022499996, 0, 4, 0, 0, 1, 0, 2, 0, 3,
      1};
  cudaq::spin_op H(h2_data);
  const auto param_space = cudaq::linspace(-M_PI, M_PI, 10);
  printf("Using the hand-coded kernel\n");
  for (const auto &param : param_space) {
    // `E(params...) = <psi(params...) | H | psi(params...)>`
    double energy_at_param = cudaq::observe(ansatz_handcoded{}, H, param);
    printf("<H>(%lf) = %lf\n", param, energy_at_param);
  }

  printf("Using the kernel written with cudaq::compute_action\n");
  for (const auto &param : param_space) {
    // `E(params...) = <psi(params...) | H | psi(params...)>`
    double energy_at_param =
        cudaq::observe(ansatz_compute_action{}, H, std::vector<double>{param});
    printf("<H>(%lf) = %lf\n", param, energy_at_param);
  }
}
