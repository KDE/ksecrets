project(KSecrets)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KSECRETS
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksecrets_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SecretsConfigVersion.cmake")

set(KF5_DEP_VERSION "5.12.0")
find_package(KF5I18n ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Service ${KF5_DEP_VERSION} REQUIRED)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Secrets")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5SecretsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SecretsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SecretsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SecretsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5SecretsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5SecretsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/ksecrets_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()


set (ksecrets_SRCS
    ksecretsservice.cpp
    ksecretsitem.cpp
    ksecretsvalue.cpp
    ksecretscollection.cpp
)

add_library(KF5Secrets SHARED ${ksecrets_SRCS})
generate_export_header(KF5Secrets BASE_NAME KSecrets)
add_library(KF5::Secrets ALIAS KF5Secrets)

target_include_directories(KF5Secrets INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KSecrets>")

target_link_libraries(KF5Secrets PUBLIC
    Qt5::Widgets
    Qt5::Concurrent
    KF5::I18n
    KF5::CoreAddons
    KF5::Service
)

set_target_properties(KF5Secrets PROPERTIES VERSION   ${KSECRETS_VERSION_STRING}
                                         SOVERSION ${KSECRETS_SOVERSION}
                                         EXPORT_NAME Secrets
)

ecm_generate_headers(
    KSecrets_HEADERS
    HEADER_NAMES
        KSecretsService
        KSecretsCollection
        KSecretsItem
        KSecretsValue
    REQUIRED_HEADERS KSecrets_HEADERS
)

install(TARGETS KF5Secrets EXPORT KF5SecretsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ksecrets_export.h
  ${KSecrets_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/KSecrets COMPONENT Devel
)

INSTALL(FILES
    ksecretscollection.h
    ksecretsitem.h
    ksecretsvalue.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/ksecretsservice COMPONENT Devel
)

