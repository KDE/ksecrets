
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
find_package(LibGcrypt 1.6.0 REQUIRED)

cmake_minimum_required(VERSION 3.2.2)

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KSECRETS_BACKEND
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksecrets_store_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SecretsStoreConfigVersion.cmake")

set(ksecrets_store_SRC
    ksecrets_credentials.c
    ksecrets_store.cpp)

add_library(ksecrets_store SHARED ${ksecrets_store_SRC})
generate_export_header(ksecrets_store BASE_NAME ksecrets_store)
target_link_libraries(ksecrets_store
    pthread
    keyutils
    ${LIBGCRYPT_LIBRARIES})
target_compile_features(ksecrets_store PRIVATE cxx_range_for)
set_target_properties(ksecrets_store PROPERTIES PREFIX "")

