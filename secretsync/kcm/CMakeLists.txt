
########### next target ###############

set(kcm_ksecretsync_PART_SRCS 
    kcsecretsyncmodule.cpp 
    configwidget.cpp 
    addcomputerdialog.cpp
)

kde4_add_ui_files(kcm_ksecretsync_PART_SRCS 
    configwidget.ui
    addcomputerwidget.ui
)

kde4_add_kcfg_files(kcm_ksecretsync_PART_SRCS
    ksecretsynccfg.kcfgc
)

kde4_add_plugin(kcm_ksecretsync ${kcm_ksecretsync_PART_SRCS})


target_link_libraries(kcm_ksecretsync  
    ${KDE4_KDEUI_LIBS}
    ${QCA2_LIBRARIES}
)

install(TARGETS kcm_ksecretsync  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kcm_ksecretsync.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kcm_ksecretsync.kcfg DESTINATION ${KCFG_INSTALL_DIR})

