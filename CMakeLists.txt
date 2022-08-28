cmake_minimum_required(VERSION 3.23)
project(default_systemc)

set(CMAKE_CXX_STANDARD 17)

add_executable(default_systemc main.cpp custom_structs/vec3.hpp custom_structs/triangle.hpp custom_structs/intersection.hpp custom_structs/ray.hpp modules/ist.hpp modules/testbench.hpp custom_structs/bvh.hpp custom_structs/bounding_box.hpp modules/trv.hpp custom_structs/aabb_intersector.hpp)
target_link_libraries(default_systemc systemc)
