import cudaq
from cudaq import operators, spin, Schedule, ScipyZvodeIntegrator
from cudaq.operator import coherent_state
import numpy as np
import cupy as cp
import os
import matplotlib.pyplot as plt

# Set the target to our dynamics simulator
cudaq.set_target("dynamics")

# This example demonstrates a simulation of a superconducting transmon qubit coupled to a resonator (i.e., cavity).
# For reference, see "Charge-insensitive qubit design derived from the Cooper pair box", PRA 76, 042319

# Number of cavity photons
N = 20

# System dimensions: transmon + cavity
dimensions = {0: 2, 1: N}

# See III.B of PRA 76, 042319
# System parameters
# Unit: GHz
omega_01 = 3.0 * 2 * np.pi  # transmon qubit frequency
omega_r = 2.0 * 2 * np.pi  # resonator frequency
# Dispersive shift
chi_01 = 0.025 * 2 * np.pi
chi_12 = 0.0

# Alias for commonly used operators
# Cavity operators
a = operators.annihilate(1)
a_dag = operators.create(1)
nc = operators.number(1)
xc = operators.annihilate(1) + operators.create(1)

# Transmon operators
sz = spin.z(0)
sx = spin.x(0)
nq = operators.number(0)
xq = operators.annihilate(0) + operators.create(0)

# See equation 3.8
omega_01_prime = omega_01 + chi_01
omega_r_prime = omega_r - chi_12 / 2.0
chi = chi_01 - chi_12 / 2.0
hamiltonian = 0.5 * omega_01_prime * sz + (omega_r_prime + chi * sz) * a_dag * a

# Initial state of the system
# Transmon in a superposition state
transmon_state = cp.array([1. / np.sqrt(2.), 1. / np.sqrt(2.)],
                          dtype=cp.complex128)
# Cavity in a superposition state
cavity_state = coherent_state(N, 2.0)
psi0 = cudaq.State.from_data(cp.kron(transmon_state, cavity_state))

steps = np.linspace(0, 250, 1000)
schedule = Schedule(steps, ["time"])

# Evolve the system
evolution_result = cudaq.evolve(hamiltonian,
                                dimensions,
                                schedule,
                                psi0,
                                observables=[nc, nq, xc, xq],
                                collapse_operators=[],
                                store_intermediate_results=True,
                                integrator=ScipyZvodeIntegrator())

get_result = lambda idx, res: [
    exp_vals[idx].expectation() for exp_vals in res.expectation_values()
]
count_results = [
    get_result(0, evolution_result),
    get_result(1, evolution_result)
]

quadrature_results = [
    get_result(2, evolution_result),
    get_result(3, evolution_result)
]

fig = plt.figure(figsize=(18, 6))

plt.subplot(1, 2, 1)
plt.plot(steps, count_results[0])
plt.plot(steps, count_results[1])
plt.ylabel("n")
plt.xlabel("Time [ns]")
plt.legend(("Cavity Photon Number", "Transmon Excitation Probability"))
plt.title("Excitation Numbers")

plt.subplot(1, 2, 2)
plt.plot(steps, quadrature_results[0])
plt.ylabel("x")
plt.xlabel("Time [ns]")
plt.legend(("cavity"))
plt.title("Resonator Quadrature")

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)
fig.savefig('transmon_resonator.png', dpi=fig.dpi)
