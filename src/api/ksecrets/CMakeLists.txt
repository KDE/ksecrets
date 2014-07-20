project( KSecrets )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)

include_directories(${QCA2_INCLUDE_DIR})

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KSECRETS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksecrets_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SecretsConfigVersion.cmake")

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Secrets")

include(CMakePackageConfigHelpers)

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
    ksecretsserviceitem.cpp
    ksecretsserviceitemjobs.cpp
    ksecretsservicesecret.cpp
    ksecretsservicecollection.cpp
    ksecretsservicecollectionjobs.cpp
    ksecretsservicecodec.cpp
    dbusbackend.cpp
    promptjob.cpp
)

set_source_files_properties( 
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Collection.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Item.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Prompt.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Service.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Session.xml
    PROPERTIES INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/ksecretsservicedbustypes.h )

qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Collection.xml collection_interface)
qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Item.xml item_interface)
qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Prompt.xml prompt_interface)
qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Service.xml service_interface)
qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Session.xml session_interface)

add_library(KF5Secrets SHARED ${ksecrets_SRCS})
generate_export_header(KF5Secrets BASE_NAME KSecrets)
add_library(KF5::Secrets ALIAS KF5Secrets)

target_include_directories(KF5Secrets INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KSecrets>")

target_link_libraries(KF5Secrets PUBLIC
    Qt5::Widgets
    Qt5::DBus
    ${QCA2_LIBRARIES}
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
        KSecretsServiceCollection
    REQUIRED_HEADERS KSecrets_HEADERS
)

#install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/KSecrets DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS KF5Secrets EXPORT KF5SecretsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ksecrets_export.h
  ${KSecrets_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/KSecrets COMPONENT Devel
)

INSTALL(FILES 
    ${ksecretsservice_xml} 
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

# INSTALL(FILES
#     ksecretsservicecollection.h
#     ksecretsservicecollectionjobs.h
#     ksecretsserviceitem.h
#     ksecretsserviceitemjobs.h
#     ksecretsservicesecret.h
#     ksecretsservicedbustypes.h
#     ksecretsservicecodec.h
#     DESTINATION ${INCLUDE_INSTALL_DIR}/ksecretsservice COMPONENT Devel 
# )
# 
# INSTALL(FILES
#     ksecretsservicedbustypes.h
#     ksecretsservicecodec.h
#     DESTINATION ${INCLUDE_INSTALL_DIR}/ksecretsservice/private COMPONENT Devel 
# )

if(BUILD_TESTS)
add_subdirectory(tests)
endif()
