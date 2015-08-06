
project(ksecrets_backend)
cmake_minimum_required(VERSION 3.2.2)

set(ksecrets_backend_SRC
   ksecrets_backend.cpp)

add_library(ksecrets_backend SHARED ${ksecrets_backend_SRC})
target_link_libraries(ksecrets_backend pthread)
target_compile_features(ksecrets_backend PRIVATE cxx_range_for)
set_target_properties(ksecrets_backend PROPERTIES PREFIX "")

