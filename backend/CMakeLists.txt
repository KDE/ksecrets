
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR})

ADD_SUBDIRECTORY (tests)

SET (ksecretservice_backend_SRCS
    # Backend
    backendjob.cpp
    backendcollection.cpp
    backendcollectionmanager.cpp
    backenditem.cpp
    backendmaster.cpp
    securebuffer.cpp
    temporary/temporarycollection.cpp
    temporary/temporarycollectionmanager.cpp
    temporary/temporaryitem.cpp
    temporary/temporaryjobs.cpp
    ksecret/ksecretcollectionmanager.cpp
    ksecret/ksecretcollection.cpp
    ksecret/ksecretitem.cpp
    ksecret/ksecretjobs.cpp
    ksecret/ksecretencryptionfilter.cpp
    ksecret/ksecretstream.cpp
    ../peer.cpp
)

KDE4_ADD_LIBRARY (ksecretservicebackend STATIC ${ksecretservice_backend_SRCS})
TARGET_LINK_LIBRARIES (ksecretservicebackend ${KDE4_KDECORE_LIBS} ${QCA2_LIBRARIES})
