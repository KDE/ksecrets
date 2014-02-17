project( ksecretsserviced )

include_directories(
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../ksecretsservice
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)

add_subdirectory( lib )
add_subdirectory( ui )
add_subdirectory( backend )
add_subdirectory( frontend )

#
# ksecretserviced
# 

add_executable( ksecretsserviced
    main.cpp
    peer.cpp
)

target_link_libraries( ksecretsserviced
    ksecretservicebackend
    ksecretservicefrontend
    ksecretserviceui
    KF5::Secrets
    KF5::KDE4Support
)

install( TARGETS ksecretsserviced ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES ksecretsserviced.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
configure_file( org.kde.ksecretsserviced.service.in org.kde.ksecretsserviced.service )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ksecretsserviced.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR} )
