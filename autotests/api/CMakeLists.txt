
include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include_directories(
   ${CMAKE_SOURCE_DIR}/src/runtime/ksecrets_store
   ${CMAKE_BINARY_DIR}/src/runtime/ksecrets_store
   )

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

find_package(KF5 REQUIRED CoreAddons)

ecm_add_test(
    ksecretsservice-test.cpp
    LINK_LIBRARIES Qt5::Test KF5::Secrets KF5::CoreAddons ksecrets_store keyutils
)

