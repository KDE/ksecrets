
set( secretsyncapplet_SRC
    secretsyncapplet.cpp
)

kde4_add_plugin(plasma_applet_secretsync ${secretsyncapplet_SRC})

target_link_libraries(plasma_applet_secretsync 
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
)

install(TARGETS plasma_applet_secretsync DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma_applet_secretsync.desktop DESTINATION ${SERVICES_INSTALL_DIR})
