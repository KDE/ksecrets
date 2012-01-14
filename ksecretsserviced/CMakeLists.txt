PROJECT (ksecretsserviced)

include(MacroLibrary)
include(KDE4Defaults)

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../ksecretsservice
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)

ADD_SUBDIRECTORY (lib)
ADD_SUBDIRECTORY (ui)
ADD_SUBDIRECTORY (backend)
ADD_SUBDIRECTORY (frontend)

#
# ksecretserviced
# 

KDE4_ADD_EXECUTABLE (ksecretsserviced 
    main.cpp
    peer.cpp
)

TARGET_LINK_LIBRARIES (ksecretsserviced
    ksecretservicebackend
    ksecretservicefrontend
    ksecretserviceui
    ksecretsservice
)

install( TARGETS ksecretsserviced ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ksecretsserviced.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
configure_file(org.kde.ksecretsserviced.service.in org.kde.ksecretsserviced.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ksecretsserviced.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR} )
