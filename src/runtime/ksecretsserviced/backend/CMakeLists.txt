include_directories(
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(BUILD_TESTS)
add_subdirectory( tests )
endif()

set( ksecretservice_backend_SRCS
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

add_library( ksecretservicebackend STATIC ${ksecretservice_backend_SRCS} )

target_link_libraries( ksecretservicebackend
    daemonlib
    ksecretserviceui
    KF5::CoreAddons
    KF5::I18n
    KF5::KDE4Support #KGlobal+KStandardDirs
    Qt5::Core
    Qt5::Widgets
    ${QCA2_LIBRARIES}
)
