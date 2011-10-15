
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR})


ADD_SUBDIRECTORY (tests)

SET (ksecretservice_frontend_SRCS
    # fd.o Secret Storage daemon
    secret/item.cpp
    secret/collection.cpp
    secret/prompt.cpp
    secret/service.cpp
    secret/session.cpp
    # Daemon DBus adaptors
    secret/adaptors/daemonsecret.cpp
    secret/adaptors/dbustypes.cpp
    secret/adaptors/collectionadaptor.cpp
    secret/adaptors/itemadaptor.cpp
    secret/adaptors/promptadaptor.cpp
    secret/adaptors/serviceadaptor.cpp
    secret/adaptors/sessionadaptor.cpp
    secret/adaptors/daemonsecret.cpp
)

KDE4_ADD_LIBRARY (ksecretservicefrontend STATIC ${ksecretservice_frontend_SRCS})
TARGET_LINK_LIBRARIES (ksecretservicefrontend 
    ${KDE4_KDECORE_LIBS} 
    ${QCA2_LIBRARIES}
)

INSTALL(FILES 
    secret/adaptors/daemonsecret.h
    secret/adaptors/dbustypes.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/ksecretsservice COMPONENT Devel 
)
