
include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)
find_package(LibGcrypt 1.6.0 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions" )

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)

ecm_add_test(
    ksecrets_store_test.cpp
    LINK_LIBRARIES Qt5::Test KF5::CoreAddons KF5::ConfigCore ksecrets_store
)

ecm_add_test(
    crypt_buffer_test.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/crypt_buffer.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/ksecrets_store.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/ksecrets_file.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/ksecrets_data.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/crypting_engine.cpp
    LINK_LIBRARIES Qt5::Test KF5::CoreAddons KF5::ConfigCore ksecrets_store ${LIBGCRYPT_LIBRARIES}
    TEST_NAME crypt_buffer_test
)

ecm_add_test(
    ksecrets_file_test.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/ksecrets_file.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/ksecrets_data.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/crypt_buffer.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/ksecrets_store.cpp
    ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store/crypting_engine.cpp
    LINK_LIBRARIES Qt5::Test KF5::CoreAddons KF5::ConfigCore ksecrets_store ${LIBGCRYPT_LIBRARIES}
    TEST_NAME ksecrets_file_test
)
