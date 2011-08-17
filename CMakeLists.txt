PROJECT( kio_ksecretsservice )

FIND_PACKAGE( KDE4 REQUIRED )

INCLUDE_DIRECTORIES( ${KDE4_INCLUDES} ${CMAKE_SOURCE_DIR} )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)

set( kio_ksecretsservice_SRCS
    kio_ksecretsservice.cpp
)

KDE4_ADD_PLUGIN( kio_ksecretsservice ${kio_ksecretsservice_SRCS} )

TARGET_LINK_LIBRARIES( kio_ksecretsservice 
    ${KDE4_KIO_LIBS} 
    ksecretsserviceclient
)

INSTALL( TARGETS kio_ksecretsservice DESTINATION ${PLUGIN_INSTALL_DIR} )

INSTALL( FILES secrets.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
