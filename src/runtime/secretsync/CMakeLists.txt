PROJECT (ksecretsync)

include(MacroLibrary)
include(KDE4Defaults)

set(KSS_DBUS_INTERFACES_DIR ../ksecretsservice)

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)
#
# ksecret-sync
#
add_subdirectory(icons)
add_subdirectory(kcm)
#add_subdirectory(applet)
if(BUILD_TESTS)
add_subdirectory(tests)
endif()

SET( ksecretsync_SRCS 
    statuswidget.cpp 
    syncdaemon.cpp 
    syncjob.cpp 
    computersyncjob.cpp 
    ksecretsyncwindow.cpp 
    trayicon.cpp
    main.cpp
    syncmodel.cpp
    syncserverjob.cpp
    syncprotocol.cpp
)

set_source_files_properties( 
    ${KSS_DBUS_INTERFACES_DIR}/org.freedesktop.Secret.Service.xml
    ${KSS_DBUS_INTERFACES_DIR}/org.freedesktop.Secret.Session.xml
    PROPERTIES INCLUDE ../ksecretsservice/ksecretsservicedbustypes.h )

QT4_ADD_DBUS_INTERFACE(ksecretsync_SRCS  ${KSS_DBUS_INTERFACES_DIR}/org.freedesktop.Secret.Service.xml service_interface )
QT4_ADD_DBUS_INTERFACE(ksecretsync_SRCS  ${KSS_DBUS_INTERFACES_DIR}/org.freedesktop.Secret.Session.xml session_interface )

kde4_add_kcfg_files(ksecretsync_SRCS
     kcm/ksecretsynccfg.kcfgc
)

KDE4_ADD_UI_FILES ( ksecretsync_SRCS
    statuswidget.ui
)

KDE4_ADD_EXECUTABLE (ksecretsync ${ksecretsync_SRCS})

TARGET_LINK_LIBRARIES (ksecretsync
    ${KDE4_KDEUI_LIBS}
    ${QCA2_LIBRARIES}
    ${QT_QTNETWORK_LIBRARY}
)

install( TARGETS ksecretsync ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ksecretsync.rc DESTINATION ${DATA_INSTALL_DIR}/ksecretsync)

kde4_install_icons( ${ICON_INSTALL_DIR} )


install(DIRECTORY plasma/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/secretsync-qml PATTERN *~ EXCLUDE)
 
install(FILES plasma/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-secretsync.desktop)
