
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
find_package(LibGcrypt 1.6.0 REQUIRED)

cmake_minimum_required(VERSION 3.2.2)

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KSECRETS_BACKEND
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksecrets_backend_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SecretsBackendConfigVersion.cmake")

set(ksecrets_backend_SRC
   ksecrets_crypt.c
   ksecrets_backend.cpp)

add_library(ksecrets_backend SHARED ${ksecrets_backend_SRC})
generate_export_header(ksecrets_backend BASE_NAME ksecrets_backend)
target_link_libraries(ksecrets_backend
   pthread
   ${LIBGCRYPT_LIBRARIES})
target_compile_features(ksecrets_backend PRIVATE cxx_range_for)
set_target_properties(ksecrets_backend PROPERTIES PREFIX "")

