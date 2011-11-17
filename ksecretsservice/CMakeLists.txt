PROJECT( ksecretsservice )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)

add_subdirectory(tests)

include_directories (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set (client_lib_SRCS
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

qt4_add_dbus_interface(client_lib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Collection.xml collection_interface)
qt4_add_dbus_interface(client_lib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Item.xml item_interface)
qt4_add_dbus_interface(client_lib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Prompt.xml prompt_interface)
qt4_add_dbus_interface(client_lib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Service.xml service_interface)
qt4_add_dbus_interface(client_lib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Session.xml session_interface)

kde4_add_library(ksecretsservice SHARED ${client_lib_SRCS})
SET_TARGET_PROPERTIES ( ksecretsservice PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

TARGET_LINK_LIBRARIES (ksecretsservice 
    ${KDE4_KDECORE_LIBS}
    ${QCA2_LIBRARIES}
)

INSTALL(TARGETS ksecretsservice EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES 
    ${ksecretsservice_xml} 
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

INSTALL(FILES
    ksecretsservicecollection.h
    ksecretsservicecollectionjobs.h
    ksecretsserviceitem.h
    ksecretsserviceitemjobs.h
    ksecretsservicesecret.h
    ksecretsservicedbustypes.h
    ksecretsservicecodec.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/ksecretsservice COMPONENT Devel 
)

INSTALL(FILES
    ksecretsservicedbustypes.h
    ksecretsservicecodec.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/ksecretsservice/private COMPONENT Devel 
)
