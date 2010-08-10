INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR})
ADD_SUBDIRECTORY (tests)

SET (ksecretservice_daemon_SRCS
   # Daemon
   item.cpp
   collection.cpp
   prompt.cpp
   secret.cpp
   service.cpp
   session.cpp

   # Daemon DBus adaptors
   dbus/dbustypes.cpp
   dbus/collectionadaptor.cpp
   dbus/itemadaptor.cpp
   dbus/promptadaptor.cpp
   dbus/serviceadaptor.cpp
   dbus/sessionadaptor.cpp
)

KDE4_ADD_LIBRARY (ksecretservicedaemon STATIC ${ksecretservice_daemon_SRCS})
TARGET_LINK_LIBRARIES (ksecretservicedaemon ${KDE4_KDECORE_LIBS} ${QCA2_LIBRARIES})
