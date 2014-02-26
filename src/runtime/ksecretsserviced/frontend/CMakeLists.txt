include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../api/ksecrets
)

if(BUILD_TESTS)
add_subdirectory( tests )
endif()

set( ksecretservice_frontend_SRCS
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

add_library( ksecretservicefrontend STATIC ${ksecretservice_frontend_SRCS} )

target_link_libraries( ksecretservicefrontend
    KF5::Secrets
    Qt5::Core
    Qt5::DBus
    KF5::WidgetsAddons
    ${QCA2_LIBRARIES}
)

install( FILES
    secret/adaptors/daemonsecret.h
    secret/adaptors/dbustypes.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/ksecretsservice COMPONENT Devel 
)
