# ============================================================================ #
# Copyright (c) 2022 - 2025 NVIDIA Corporation & Affiliates.                   #
# All rights reserved.                                                         #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

enable_language(CUDA)
find_package(CUDAToolkit REQUIRED)

set(INTERFACE_POSITION_INDEPENDENT_CODE ON)
set(LIBRARY_NAME nvqir-dynamics)

# Find cudensitymat header (for MPI plugin interface typedef)
find_file(CUDENSITYMAT_INC
    NAMES   cudensitymat.h
    HINTS   
        $ENV{CUQUANTUM_INSTALL_PREFIX}/include      
        /usr/include    
        ENV CPATH
    REQUIRED
)

message(STATUS "cudensitymat header: ${CUDENSITYMAT_INC}")
get_filename_component(CUDENSITYMAT_INCLUDE_DIR ${CUDENSITYMAT_INC} DIRECTORY)

add_library(${LIBRARY_NAME} SHARED
  CuDensityMatSim.cpp
  mpi_support.cpp
  CuDensityMatTimeStepper.cpp
  RungeKuttaIntegrator.cpp
  CuDensityMatExpectation.cpp
  CuDensityMatEvolution.cpp
  CuDensityMatState.cpp
  CuDensityMatContext.cpp
  CuDensityMatOpConverter.cpp
)

message("CUDAToolkit_INCLUDE_DIRS = ${CUDAToolkit_INCLUDE_DIRS}")
target_include_directories(${LIBRARY_NAME}
  PRIVATE 
    . .. 
    ${CUDAToolkit_INCLUDE_DIRS} 
    ${CMAKE_SOURCE_DIR}/runtime/common
    ${CUDENSITYMAT_INCLUDE_DIR}
)

target_include_directories(${LIBRARY_NAME}
  PUBLIC
    . ..
    ${CUDAToolkit_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/runtime/common
    ${CUDENSITYMAT_INCLUDE_DIR}
)

target_link_libraries(${LIBRARY_NAME}
                      PRIVATE
                        fmt::fmt-header-only
                        cudaq-common
                        ${CUDENSITYMAT_ROOT}/lib/libcudensitymat.so.0
                        CUDA::cudart
                     )
target_link_libraries(${LIBRARY_NAME}
                     PUBLIC
                       cudaq-operator
                       ${CUDENSITYMAT_ROOT}/lib/libcudensitymat.so.0
                    )
install(TARGETS ${LIBRARY_NAME} DESTINATION lib)

