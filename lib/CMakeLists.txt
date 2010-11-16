INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR})

ADD_SUBDIRECTORY (tests)

SET (ksecretservice_lib_SRCS
   jobqueue.cpp
   queuedjob.cpp
   secrettool.cpp
   peer.cpp
)

KDE4_ADD_LIBRARY (ksecretservicelib STATIC ${ksecretservice_lib_SRCS})
TARGET_LINK_LIBRARIES (ksecretservicelib ${QT_QTCORE_LIBRARY})

