SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES (
   ${KDE4_INCLUDES}
   ${QCA2_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
)

KDE4_ADD_EXECUTABLE (ksecretservice_daemon_test
   servicetest.cpp
   tempblockingcollectionmanager.cpp
   tempblockingcollection.cpp
   tempblockingitem.cpp
)
TARGET_LINK_LIBRARIES (ksecretservice_daemon_test
   ksecretservicebackend
   ksecretservicedaemon
   ${QT_QTTEST_LIBRARIES}
)

ADD_TEST (DaemonSessionTest ksecretservice_daemon_test)
