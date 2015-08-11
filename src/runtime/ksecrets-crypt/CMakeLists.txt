project(ksecrets-crypt)
cmake_minimum_required(VERSION 3.2.2)

set(GCC_KSECRETS_CRYPT_FLAGS "-fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_KSECRETS_CRYPT_FLAGS}")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
find_package(LibGcrypt 1.6.0 REQUIRED)

set(KF5_DEP_VERSION "5.12.0")
find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ksecrets_backend)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../ksecrets_backend)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
set(ksecrets_config_SRC
  config.cpp)

add_library(ksecrets_config STATIC ${ksecrets_config_SRC})
target_link_libraries(ksecrets_config PRIVATE KF5::ConfigCore)
set_target_properties(ksecrets_config PROPERTIES PREFIX "")

