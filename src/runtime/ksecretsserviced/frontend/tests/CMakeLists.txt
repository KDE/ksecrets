set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable( ksecretservice_frontend_test
    servicetest.cpp
    tempblockingcollectionmanager.cpp
    tempblockingcollection.cpp
    tempblockingitem.cpp
    tempblockingjobs.cpp
)
target_link_libraries( ksecretservice_frontend_test
    ksecretservicebackend
    ksecretservicefrontend
    ksecretserviceui
    daemonlib
    Qt5::Test
)

add_test( FrontendSessionTest ksecretservice_frontend_test )
