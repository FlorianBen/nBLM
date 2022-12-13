# ########################################################################################
# Use the code in this section for standalone projects
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(nBLM)

if(NOT TARGET Garfield::Garfield)
  find_package(Garfield REQUIRED)
endif()

# ########################################################################################

# ---Build executable------------------------------------------------------------
add_executable(nblm nblm.cpp)
target_link_libraries(nblm Garfield::Garfield)

# ---Build executable------------------------------------------------------------
add_executable(gen_gas gen_gas.cpp)
target_link_libraries(gen_gas Garfield::Garfield)

# ---Copy all files locally to the build directory-------------------------------
foreach(_file data/nBLM_dielectrics.dat data/nBLM/nBLM_field_17.txt data/nBLM/nBLM_ramo_17.txt data/nBLM/nBLM_mesh_17.mphtxt)
  configure_file(${_file} ${_file} COPYONLY)
endforeach()