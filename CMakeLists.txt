cmake_minimum_required(VERSION 2.8.3)
project(emoa)

# find_package(Boost REQUIRED)

set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS}")
set(CPLEX_DIR /opt/ibm/ILOG/CPLEX_Studio_Community2211)
set(GUROBI_DIR gurobi1003/linux64)
# set(orDIR /home/anushtup/Documents/or-tools_amd64_ubuntu-20.04_cpp_v9.8.3296/or-tools_x86_64_Ubuntu-20.04_cpp_v9.8.3296)
# set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};<path/to/ortools>/lib/cmake/ortools")


file(GLOB_RECURSE ALL_HDRS "include/*.hpp")
file(GLOB_RECURSE ALL_SRCS "source/*.cpp")

include_directories(DIRECTORY 
  include/${PROJECT_NAME}
  include
  ${CPLEX_DIR}/cplex/include
  ${CPLEX_DIR}/concert/include
  ${GUROBI_DIR}/include
  # ${orDIR}/include
)

link_directories(
  ${CPLEX_DIR}/cplex/lib/x86-64_linux/static_pic
  ${CPLEX_DIR}/concert/lib/x86-64_linux/static_pic
  ${GUROBI_DIR}/lib
  # ${orDIR}/lib
  # ${orDIR}/lib/cmake/ortools
)

# find_package(ortools CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED
  ${ALL_SRCS}
)

set(test_cpp_dir "test/")
set(test_cpp_files "")
file(GLOB_RECURSE test_cpp_files "${test_cpp_dir}/*.cpp")
foreach(test_cpp_file ${test_cpp_files})
  get_filename_component(test_cpp_name ${test_cpp_file} NAME_WE)
  #ADD_EXECUTABLE(${test_cpp_name} ${ALL_SRCS} ${test_cpp_dir}/${test_cpp_name}.cpp)
  ADD_EXECUTABLE(${test_cpp_name} ${test_cpp_dir}/${test_cpp_name}.cpp)
  TARGET_LINK_LIBRARIES(${test_cpp_name} 
    ${PROJECT_NAME} 
    # pthread ## this is for linux systems
    ilocplex
    cplex
    concert
    m
    pthread
    dl
    gurobi100
    gurobi_g++5.2
    # ortools
  )
endforeach(test_cpp_file ${test_cpp_files})
