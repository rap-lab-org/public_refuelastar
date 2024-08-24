cmake_minimum_required(VERSION 2.8.3)
project(emoa)

# find_package(Boost REQUIRED)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
if (APPLE)
  set(GUROBI_DIR /Library/gurobi1103/macos_universal2)
  set(GRBLIB "gurobi110")
else()
  # using the 
  set(GUROBI_DIR gurobi_dir)
  set(GRBLIB "gurobi100")
endif()
# set(orDIR /home/anushtup/Documents/or-tools_amd64_ubuntu-20.04_cpp_v9.8.3296/or-tools_x86_64_Ubuntu-20.04_cpp_v9.8.3296)
# set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};<path/to/ortools>/lib/cmake/ortools")


file(GLOB_RECURSE ALL_HDRS "include/*.hpp")
file(GLOB_RECURSE ALL_SRCS "source/*.cpp")

include_directories(DIRECTORY 
  include/${PROJECT_NAME}
  include
  ${GUROBI_DIR}/include
)

link_directories( ${GUROBI_DIR}/lib)

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
  )
endforeach(test_cpp_file ${test_cpp_files})



set(exec_cpp_dir "program/")
set(exec_cpp_files "")
file(GLOB_RECURSE exec_cpp_files "${exec_cpp_dir}/*.cpp")
foreach(exec_cpp_file ${exec_cpp_files})
  get_filename_component(exec_cpp_name ${exec_cpp_file} NAME_WE)
  ADD_EXECUTABLE(${exec_cpp_name} ${exec_cpp_dir}/${exec_cpp_name}.cpp)
  TARGET_LINK_LIBRARIES(${exec_cpp_name} 
    ${PROJECT_NAME} 
    # ${GRBLIB}
    # gurobi_c++
  )
endforeach(exec_cpp_file ${exec_cpp_files})

target_link_libraries("mip-gurobi" 
  ${PROJECT_NAME}
  ${GRBLIB}
  gurobi_c++
)
