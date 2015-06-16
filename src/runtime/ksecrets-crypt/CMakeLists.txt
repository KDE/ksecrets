project(ksecrets-crypt)
cmake_minimum_required(VERSION 3.2.2)

set(GCC_KSECRETS_CRYPT_FLAGS "-fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_KSECRETS_CRYPT_FLAGS}")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )
find_package(LibGcrypt 1.6.0 REQUIRED)

set(ksecrets-crypt_SRC
  ksecrets-crypt.c)

add_library(ksecrets-crypt STATIC ${ksecrets-crypt_SRC})
set_target_properties(ksecrets-crypt PROPERTIES PREFIX "")
target_link_libraries(ksecrets-crypt
  ${LIBGCRPYT_LIBRARIES})

