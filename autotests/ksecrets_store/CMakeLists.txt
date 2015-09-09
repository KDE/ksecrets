
include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include_directories(${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store)

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
    LINK_LIBRARIES Qt5::Test KF5::CoreAddons KF5::ConfigCore ksecrets_store
)
