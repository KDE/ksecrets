PROJECT (ksecretsync)

FIND_PACKAGE (QCA2 REQUIRED)
FIND_PACKAGE (DBus REQUIRED)

add_definitions(${DBUS_DEFINITIONS})

include(MacroLibrary)
include(KDE4Defaults)

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)


#
# ksecret-sync
#
add_subdirectory(icons)

SET( ksecretsync_SRCS 
    ksecretsync.cpp 
    trayicon.cpp
    main.cpp
)

KDE4_ADD_EXECUTABLE (ksecretsync ${ksecretsync_SRCS})

TARGET_LINK_LIBRARIES (ksecretsync
    ${KDE4_KDEUI_LIBS}
    ${QCA2_LIBRARIES}
)


install( TARGETS ksecretsync ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ksecretsync.rc DESTINATION ${DATA_INSTALL_DIR}/ksecretsync)

kde4_install_icons( ${ICON_INSTALL_DIR} )

