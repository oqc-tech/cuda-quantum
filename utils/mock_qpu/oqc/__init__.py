#== == == == == == == == == == == == == == == == == == == == == == == == == == \
    == == == == == == == == == == == == #
#Copyright(c) 2022 - 2023 NVIDIA Corporation &Affiliates.#
#All rights reserved.#
# #
#This source code and the accompanying materials are made available under #
#the terms of the Apache License 2.0 which accompanies this distribution.#
#== == == == == == == == == == == == == == == == == == == == == == == == == == \
    == == == == == == == == == == == == #
import cudaq

    from fastapi import FastAPI, Request, HTTPException, Header from typing import Optional, Union import uvicorn, uuid, json, base64, ctypes from pydantic import BaseModel from llvmlite import binding as llvm

#Define the REST Server App
    app = FastAPI()

#Jobs look like the following type
        class Task(BaseModel) :task_id:str program:str compiler_config: int

#Keep track of Job Ids to their Names
    createdJobs = {}

#Could how many times the client has requested the Job
                   countJobGetRequests = 0

                   llvm.initialize() llvm.initialize_native_target() llvm.initialize_native_asmprinter() target = llvm.Target.from_default_triple() targetMachine = target.create_target_machine() backing_mod = llvm.parse_assembly("") engine = llvm.create_mcjit_compiler(backing_mod, targetMachine)

                                                                                                                                                                                                                                                                                 def getKernelFunction(module) : for f in module.functions: if not f.is_declaration: return f return None

                                                                                                                                                                                                                                                                                                       def getNumRequiredQubits(function) : for a in function.attributes: if "requiredQubits" in str(a) : return int(str(a).split("requiredQubits\"=")[- 1].split(" ")[0].replace("\"", ""))

#Here we test that the login endpoint works
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      @app.post("/auth") async def login(username, password) : return {"token" : "auth_token" }

#Here we expose a way to post jobs,
#Must have a Access Token, Job Program must be Adaptive Profile
#with EntryPoint tag
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       @app.post("/task/submit") async def postJob(job:Job, token:Union[str, None] = Header(alias = "Authorization", default = None)) :global createdJobs, shots

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        if 'token' == None:raise HTTPException(status_code(401), detail = "Credentials not provided")

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     newId = job.task_id program = job.program decoded = base64.b64decode(program) m = llvm.module.parse_bitcode(decoded) mstr = str(m) assert('EntryPoint' in mstr)

#Get the function, number of qubits, and kernel name
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              function = getKernelFunction(m) if function == None:raise Exception("Could not find kernel function") numQubitsRequired = getNumRequiredQubits(function) kernelFunctionName = function.name

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             print("Kernel name = ", kernelFunctionName) print("Requires {} qubits".format(numQubitsRequired))

#JIT Compile and get Function Pointer
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   engine.add_module(m) engine.finalize_object() engine.run_static_constructors() funcPtr = engine.get_function_address(kernelFunctionName) kernel = ctypes.CFUNCTYPE(None)(funcPtr)

#Invoke the Kernel
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                cudaq.testing.toggleBaseProfile() qubits, context = cudaq.testing.initialize(numQubitsRequired, job.count) kernel() results = cudaq.testing.finalize(qubits, context) results.dump() createdJobs[newId] =(name, results)

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    engine.remove_module(m)

#Job "created", return the id
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        return {"job" :newId }

#Retrieve the job, simulate having to wait by counting to 3
#until we return the job results
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                @app.get("/task/{jobId}/results") async def getJob(jobId:str) :global countJobGetRequests, createdJobs, shots

#Simulate asynchronous execution
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   if countJobGetRequests < 3:
        countJobGetRequests += 1
        return {"results": None}

    countJobGetRequests = 0
    name, counts = createdJobs[jobId]
    retData = []
    for bits, count in counts.items():
        retData += [bits] * count

    res = {"results": {retData}}
    return res

@app.get("/task")
async def getJob(n = 1):
    return {"task_ids": [uuid() for _ in range(n)]}


def startServer(port):
    uvicorn.run(app, port=port, host='0.0.0.0', log_level="info")


if __name__ == '__main__':
    startServer(62454)
