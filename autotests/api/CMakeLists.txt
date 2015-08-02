
include(ECMMarkAsTest)

include(ECMAddTests)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

find_package(KF5 REQUIRED CoreAddons)

ecm_add_test(
    ksecretsservice-test.cpp
    LINK_LIBRARIES KF5::Secrets Qt5::Test KF5::Secrets KF5::CoreAddons
)

