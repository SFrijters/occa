cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

#---[ CMake Config ]--------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message(STATUS "Using CMake version ${CMAKE_VERSION}")
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0056 NEW)
#=======================================

#---[ Build Config ]--------------------
project(occa
  DESCRIPTION  "JIT Compilation for Multiple Architectures: C++, OpenMP, CUDA, HIP, OpenCL, Metal"
  HOMEPAGE_URL "https://github.com/libocca/occa"
  LANGUAGES    C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(ENABLE_OPENMP "Build with OpenMP if available" ON)
option(ENABLE_CUDA   "Build with CUDA if available"   ON)
option(ENABLE_OPENCL "Build with OpenCL if available" ON)
option(ENABLE_HIP    "Build with HIP if available"    ON)
option(ENABLE_METAL  "Build with Metal if available"  ON)
option(ENABLE_MPI    "Build with MPI if available"    ON)

option(ENABLE_TESTS    "Build tests"               OFF)
option(ENABLE_EXAMPLES "Build simple examples"     OFF)
option(ENABLE_FORTRAN  "Enable Fortran interface"  OFF)

option(ENABLE_SHARED   "Enable shared libraries" ON)
option(ENABLE_STATIC   "Enable static libraries" OFF)

if (NOT ENABLE_SHARED AND NOT ENABLE_STATIC)
  message(FATAL_ERROR "Select at least one of ENABLE_SHARED or ENABLE_STATIC")
endif()

if(ENABLE_FORTRAN)
  enable_language(Fortran)
endif()

set(OCCA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(OCCA_BUILD_DIR  ${CMAKE_BINARY_DIR})

set(OCCA_USING_VS OFF)
set(OCCA_UNSAFE   OFF)

# Test Apple first because UNIX==true for Apple and Linux.
if(APPLE)
  set(OCCA_OS "OCCA_MACOS_OS")
elseif(UNIX)
  set(OCCA_OS "OCCA_LINUX_OS")
else()
  set(OCCA_OS "OCCA_WINDOWS_OS")
endif()

include(SetCompilerFlags)
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-fno-strict-aliasing" COMPILER_SUPPORTS_NO_STRICT_ALIASING)
if(COMPILER_SUPPORTS_NO_STRICT_ALIASING)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
endif()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)
message("-- System     : ${CMAKE_SYSTEM}")
message("-- Build type : ${CMAKE_BUILD_TYPE}")
message("-- C flags    : ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
message("-- CXX flags  : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
if(ENABLE_FORTRAN)
  message("-- F90 flags  : ${CMAKE_Fortran_FLAGS} ${CMAKE_Fortran_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
endif()

if(ENABLE_FORTRAN)
  set(CMAKE_Fortran_MODULE_DIRECTORY ${OCCA_BUILD_DIR}/mod)
endif()
#=======================================

if (ENABLE_SHARED)
  #---[ libocca.so ]----------------------
  message("-- Adding shared library")
  add_library(libocca SHARED)

  # Without this, CMake will create liblibocca.so
  set_target_properties(libocca PROPERTIES
    OUTPUT_NAME occa
    LIBRARY_OUTPUT_DIRECTORY ${OCCA_BUILD_DIR}/lib)
endif()

if (ENABLE_STATIC)
  #---[ libocca_static.a -----------------
  message("-- Adding static library")
  add_library(libocca_static STATIC)

  # Without this, CMake will create liblibocca_static.a
  set_target_properties(libocca_static PROPERTIES
    OUTPUT_NAME occa_static
    LIBRARY_OUTPUT_DIRECTORY ${OCCA_BUILD_DIR}/lib)
endif()

# Find needed and requested packages
find_package(Threads REQUIRED)

if (ENABLE_SHARED)
  target_link_libraries(libocca PRIVATE
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

  target_include_directories(libocca PUBLIC
    $<BUILD_INTERFACE:${OCCA_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${OCCA_BUILD_DIR}/include>)

  target_include_directories(libocca PRIVATE
    $<BUILD_INTERFACE:${OCCA_SOURCE_DIR}/src>)

  target_compile_definitions(libocca PRIVATE -DUSE_CMAKE)
endif()

if (ENABLE_STATIC)
  target_link_libraries(libocca_static PRIVATE
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

  target_include_directories(libocca_static PUBLIC
    $<BUILD_INTERFACE:${OCCA_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${OCCA_BUILD_DIR}/include>)

  target_include_directories(libocca_static PRIVATE
    $<BUILD_INTERFACE:${OCCA_SOURCE_DIR}/src>)

  target_compile_definitions(libocca_static PRIVATE -DUSE_CMAKE)
endif()

#=======================================

#---[ OpenMP ]--------------------------
if(ENABLE_OPENMP)
  find_package(OpenMP)

  if(OPENMP_CXX_FOUND)
    set(OCCA_OPENMP_ENABLED 1)

    message("-- OpenMP include dirs: ${OpenMP_CXX_INCLUDE_DIRS}")
    message("-- OpenMP libraries:    ${OpenMP_CXX_LIBRARIES}")

    if (ENABLE_SHARED)
      target_include_directories(libocca PRIVATE ${OpenMP_CXX_INCLUDE_DIRS})
      target_link_libraries(libocca PRIVATE ${OpenMP_CXX_LIBRARIES})
    endif()
    if (ENABLE_STATIC)
      target_include_directories(libocca_static PRIVATE ${OpenMP_CXX_INCLUDE_DIRS})
      target_link_libraries(libocca_static PRIVATE ${OpenMP_CXX_LIBRARIES})
    endif()
  else(OPENMP_CXX_FOUND)
    set(OCCA_OPENMP_ENABLED 0)
  endif(OPENMP_CXX_FOUND)
endif(ENABLE_OPENMP)
#=======================================

#---[ CUDA ]----------------------------
if(ENABLE_CUDA)
  find_package(CUDA)

  if(CUDA_FOUND)
    set(OCCA_CUDA_ENABLED 1)

    # Find the shared library rather than the static that find_package returns
    find_library(CUDART_LIB
      NAMES         cudart
      PATHS         ${CUDA_TOOLKIT_ROOT_DIR}
      PATH_SUFFIXES lib64 lib
      DOC           "CUDA RT lib location")

    message("-- CUDA include dirs: ${CUDA_INCLUDE_DIRS}")
    message("-- CUDA libraries:    ${CUDART_LIB};cuda}")

    if (ENABLE_SHARED)
      target_include_directories(libocca PRIVATE ${CUDA_INCLUDE_DIRS})
      target_link_libraries(libocca PRIVATE ${CUDART_LIB} cuda)
    endif()
    if (ENABLE_STATIC)
      target_include_directories(libocca_static PRIVATE ${OpenMP_CXX_INCLUDE_DIRS})
      target_link_libraries(libocca_static PRIVATE ${OpenMP_CXX_LIBRARIES})
    endif()
  else (CUDA_FOUND)
    set(OCCA_CUDA_ENABLED 0)
  endif(CUDA_FOUND)
endif(ENABLE_CUDA)
#=======================================

#---[ OpenCL ]--------------------------
if(ENABLE_OPENCL)
  # Look in some default places for OpenCL and set OPENCL_ROOT

  # Search in user specified path first
  find_path(OPENCL_ROOT
    NAMES include/CL/cl.h
    PATHS
    ENV   OPENCL_PATH
    DOC   "OPENCL root location"
    NO_DEFAULT_PATH)

  # Now search in default path
  find_path(OPENCL_ROOT
    NAMES include/CL/cl.h
    PATHS /usr/ /opt/rocm/opencl /usr/local/cuda
    DOC   "OPENCL root location")

  # Trick CMake's default OpenCL module to look in our directory
  set(ENV{AMDAPPSDKROOT} ${OPENCL_ROOT})

  find_package(OpenCL)

  if(OpenCL_FOUND)
    set(OCCA_OPENCL_ENABLED 1)

    message("-- OpenCL include dirs: ${OpenCL_INCLUDE_DIRS}")
    message("-- OpenCL libraries:    ${OpenCL_LIBRARIES}")

    if (ENABLE_SHARED)
      target_include_directories(libocca PRIVATE ${OpenCL_INCLUDE_DIRS})
      target_link_libraries(libocca PRIVATE ${OpenCL_LIBRARIES})
    endif()
    if (ENABLE_STATIC)
      target_include_directories(libocca_static PRIVATE ${OpenCL_INCLUDE_DIRS})
      target_link_libraries(libocca_static PRIVATE ${OpenCL_LIBRARIES})
    endif()
  else (OpenCL_FOUND)
    set(OCCA_OPENCL_ENABLED 0)
  endif(OpenCL_FOUND)
endif(ENABLE_OPENCL)
#=======================================

#---[ HIP ]-----------------------------
if(ENABLE_HIP)
  find_package(HIP)

  if(HIP_FOUND)
    set(OCCA_HIP_ENABLED 1)

    message("-- HIP version:      ${HIP_VERSION_STRING}")
    message("-- HIP platform:     ${HIP_PLATFORM}")
    message("-- HIP include dirs: ${HIP_INCLUDE_DIRS}")
    message("-- HIP libraries:    ${HIP_LIBRARIES}")

    if (ENABLE_SHARED)
      target_compile_definitions(libocca PRIVATE -D${HIP_RUNTIME_DEFINE})
      target_include_directories(libocca PRIVATE ${HIP_INCLUDE_DIRS})
      target_link_libraries(libocca PRIVATE ${HIP_LIBRARIES})
    endif()
    if (ENABLE_STATIC)
      target_compile_definitions(libocca_static PRIVATE -D${HIP_RUNTIME_DEFINE})
      target_include_directories(libocca_static PRIVATE ${HIP_INCLUDE_DIRS})
      target_link_libraries(libocca_static PRIVATE ${HIP_LIBRARIES})
    endif()
  else (HIP_FOUND)
    set(OCCA_HIP_ENABLED 0)
  endif(HIP_FOUND)
endif(ENABLE_HIP)
#=======================================

#---[ Metal ]---------------------------
if(ENABLE_METAL AND APPLE)

  find_library(METAL_LIBRARY Metal)
  find_library(CORE_SERVICES CoreServices)
  find_library(APP_KIT AppKit)

  if(METAL_LIBRARY AND CORE_SERVICES AND APP_KIT)
    set(OCCA_METAL_ENABLED 1)
  else()
    set(OCCA_METAL_ENABLED 0)
  endif()
endif()
#=======================================

#---[ MPI ]-----------------------------
if(ENABLE_MPI)
  find_package(MPI)

  if(MPI_FOUND)
    set(OCCA_MPI_ENABLED 1)

    message("-- MPI include dirs: ${MPI_CXX_INCLUDE_DIRS}")
    message("-- MPI libraries:    ${MPI_CXX_LIBRARIES}")
    message("-- MPI link flags:   ${MPI_CXX_LINK_FLAGS}")
    message("-- MPI executable:   ${MPIEXEC_EXECUTABLE}")
    message("-- MPI numproc flag: ${MPIEXEC_NUMPROC_FLAG}")

    if (ENABLE_SHARED)
      target_include_directories(libocca PUBLIC ${MPI_CXX_INCLUDE_DIRS})
      target_link_libraries(libocca PUBLIC ${MPI_CXX_LIBRARIES})
    endif()
    if (ENABLE_STATIC)
      target_include_directories(libocca_static PUBLIC ${MPI_CXX_INCLUDE_DIRS})
      target_link_libraries(libocca_static PUBLIC ${MPI_CXX_LIBRARIES})
    endif()
  else(MPI_FOUND)
    set(OCCA_MPI_ENABLED 0)
  endif(MPI_FOUND)
endif(ENABLE_MPI)
#=======================================

# Generate CompiledDefines from libraries we found
configure_file(
  scripts/build/compiledDefinesTemplate.hpp.in
  ${OCCA_BUILD_DIR}/include/occa/defines/compiledDefines.hpp)

install(
  FILES ${OCCA_BUILD_DIR}/include/occa/defines/compiledDefines.hpp
  DESTINATION include/occa/defines)

# Find source files
file(
  GLOB_RECURSE OCCA_SRC_cpp
  RELATIVE ${OCCA_SOURCE_DIR} "src/*.cpp")

if(OCCA_OPENMP_ENABLED)
  set_source_files_properties(${OCCA_SRC_cpp} PROPERTIES
    COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
endif()

if(ENABLE_FORTRAN)
  file(GLOB_RECURSE OCCA_SRC_f90
    RELATIVE ${OCCA_SOURCE_DIR} "src/*.f90")

  if(OCCA_OPENMP_ENABLED)
    set_source_files_properties(${OCCA_SRC_f90} PROPERTIES
      COMPILE_FLAGS ${OpenMP_Fortran_FLAGS})
  endif()

  install(CODE
    "file(GLOB public-modules ${CMAKE_Fortran_MODULE_DIRECTORY}/*.mod)\n
     file(INSTALL DESTINATION
       ${CMAKE_INSTALL_PREFIX}/include
       TYPE FILE
       FILES \${public-modules})")

  if (ENABLE_SHARED)
    set_target_properties(libocca PROPERTIES
      LINKER_LANGUAGE C)
  endif()
  if (ENABLE_STATIC)
    set_target_properties(libocca_static PROPERTIES
      LINKER_LANGUAGE C)
  endif()
endif()

if(OCCA_METAL_ENABLED)
  file(GLOB_RECURSE OCCA_SRC_metal
    RELATIVE ${OCCA_SOURCE_DIR} "src/*.mm")
endif()

set(OCCA_SRC
  ${OCCA_SRC_cpp}
  ${OCCA_SRC_f90}
  ${OCCA_SRC_metal})

if (ENABLE_SHARED)
  target_sources(libocca PRIVATE ${OCCA_SRC})

  install(TARGETS libocca DESTINATION lib)
endif()
if (ENABLE_STATIC)
  target_sources(libocca_static PRIVATE ${OCCA_SRC})

  install(TARGETS libocca_static DESTINATION lib)
endif()
install(DIRECTORY include/ DESTINATION include)

if(ENABLE_TESTS)
  include(CTest)
  add_subdirectory(tests)
endif(ENABLE_TESTS)

if(ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif(ENABLE_EXAMPLES)

add_subdirectory(bin)
