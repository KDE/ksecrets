PROJECT (ksecretsserviced)

include(MacroLibrary)
include(KDE4Defaults)
include(PkgConfigGetVar)

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
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
    ${KDE4_KDEUI_LIBS}
)


install( TARGETS ksecretsserviced ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ksecretsserviced.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
dbus_add_activation_service(org.kde.ksecretsserviced.service.in)
