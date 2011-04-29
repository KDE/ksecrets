#INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR})

SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# SET( ksecretsync_conn_test_SRCS
#     socketconnectiontest.cpp
#     ../syncprotocol.cpp
# )
# 
# kde4_add_kcfg_files(ksecretsync_conn_test_SRCS
#      ../kcm/ksecretsynccfg.kcfgc
# )
# 
# KDE4_ADD_EXECUTABLE (ksecretsync_conn_test ${ksecretsync_conn_test_SRCS})
# 
# TARGET_LINK_LIBRARIES (ksecretsync_conn_test
#     ${QT_QTTEST_LIBRARIES}
#     ${QT_QTNETWORK_LIBRARIES}
#     ${KDE4_KDEUI_LIBS}
# )
# 
# ADD_TEST (KSecretSyncConnTest ksecretsync_conn_test)

SET( ksecretsync_protocol_test_SRCS
    protocoltest.cpp
    ../syncprotocol.cpp
)

kde4_add_kcfg_files(ksecretsync_protocol_test_SRCS
     ../kcm/ksecretsynccfg.kcfgc
)

qt4_add_dbus_interface(ksecretsync_protocol_test_SRCS  ${CMAKE_SOURCE_DIR}/client/org.freedesktop.Secret.Session.xml session_interface )

KDE4_ADD_EXECUTABLE (ksecretsync_protocol_test ${ksecretsync_protocol_test_SRCS})

TARGET_LINK_LIBRARIES (ksecretsync_protocol_test
    ${QT_QTTEST_LIBRARIES}
    ${QT_QTNETWORK_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ksecretserviceclient
)

ADD_TEST (KSecretSyncProtocolTest ksecretsync_protocol_test)
