# ============================================================================ #
# Copyright (c) 2022 - 2023 NVIDIA Corporation & Affiliates.                   #
# All rights reserved.                                                         #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

# ============================================================================ #
# Copyright (c) 2022 - 2023 NVIDIA Corporation & Affiliates.                   #
# All rights reserved.                                                         #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

import cudaq
from cudaq import spin
from fastapi import FastAPI, HTTPException, Header
from typing import Union
import uvicorn, uuid, base64, ctypes
from pydantic import BaseModel
from llvmlite import binding as llvm

import logging

class VQEDef(BaseModel):
    qubits: int
    params: list
    gates: list


# Define the REST Server App
app = FastAPI()

logger = logging.getLogger()

from cudaq import make_kernel

def extract_target(gate):
    return int(gate["targets"][0]["qId"])
def extract_controls(gate):
    return int(gate["controls"][0]["qId"]), int(gate["targets"][0]["qId"])

def extract_params(parms: str, qiskit_params):
    output = []
    parms = parms.rstrip(")").lstrip("(").split(",")
    for para in parms:
        try:
            output.append(float(para))
        except:
            split_p = para.split("*")
            multipllier = float(split_p[0])
            output.append(multipllier*qiskit_params[split_p[1]])
    return output


def extract_cuda_params(kernel, qubits, params, gate: str):
    """
    U(θ, φ, λ) := Rz(φ)Ry(θ)Rz(λ)

    kernel.ry(thetas[0], qubits[1])
    """
    # index_to_gate = {0:,1:,2:}
    output = []
    parms = gate["displayArgs"]
    parms = parms.rstrip(")").lstrip("(").split(",")
    target = extract_target(gate)
    for i, para in enumerate(parms):
        try:
            multipllier = float(para)
            if i == 1:
                kernel.ry(multipllier, qubits[target])
            else:
                kernel.rz(multipllier, qubits[target])
        except:
            split_p = para.split("*")
            multipllier = float(split_p[0])
            param_index = int(split_p[-1].split("_")[-1])
            if i == 1:
                kernel.ry(multipllier*params[param_index], qubits[target])
            else:
                kernel.rz(multipllier*params[param_index], qubits[target])

    return output


def make_cudaq_kernel(qubits, params, circuit):
    kernel, params = make_kernel(list)
    qubits = kernel.qalloc(qubits)

    for gate in circuit:
        if gate.get("gate") == "U":
            # kernel.u(*extract_params(gate["displayArgs"], qubits[extract_target(gate)]))
            extract_cuda_params(kernel, qubits, params, gate)
        else:
            control, target = extract_controls(gate)
            kernel.cx(qubits[control], qubits[target])

    return kernel, params





@app.get("/ping")
async def ping():
    return "pong"

@app.post("/test")
async def post_test():
    return "thanks"


@app.post("/vqe")
async def run_vqe(vqe_def:VQEDef):
    logger.info("Setting up VQE kernel")
    kernel, params = make_cudaq_kernel(vqe_def.qubits, vqe_def.params, vqe_def.gates)
    # logger.info(kerenl)
    hamiltonian = 5.907 - 2.1433 * spin.x(0) * spin.x(1) - 2.1433 * spin.y(
    0) * spin.y(1) + .21829 * spin.z(0) - 6.125 * spin.z(1)

    optimizer = cudaq.optimizers.COBYLA()
    energy, parameter = cudaq.vqe(
        kernel=kernel,
        spin_operator=hamiltonian,
        optimizer=optimizer,
        # list of parameters has length of 1:
        parameter_count=len(vqe_def.params)
    )
    return {"energy": energy, "parameter": parameter}


def startServer(port):
    uvicorn.run(app, port=port, host='0.0.0.0', log_level="info")


if __name__ == '__main__':
    startServer(62455)
