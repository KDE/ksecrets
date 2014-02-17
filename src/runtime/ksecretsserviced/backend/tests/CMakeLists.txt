include_directories( ${CMAKE_CURRENT_BINARY_DIR} )


add_executable( temporary_collection_test temporarytest.cpp )
target_link_libraries( temporary_collection_test
    ksecretservicebackend
    ksecretserviceui
    daemonlib
    Qt5::Test
)
add_test( TemporaryCollectionTest temporary_collection_test )


add_executable( ksecret_collection_test ksecrettest.cpp )
target_link_libraries( ksecret_collection_test
    ksecretservicebackend
    ksecretserviceui
    daemonlib
    Qt5::Test
)
add_test( KSecretCollectionTest ksecret_collection_test )


add_executable( ksecret_collection_ui_test ksecretuitest.cpp )
target_link_libraries( ksecret_collection_ui_test
    ksecretservicebackend
    ksecretserviceui
    daemonlib
    Qt5::Test
)
add_test( KSecretCollectionUiTest ksecret_collection_ui_test )


add_executable( securebuffer_test securebuffertest.cpp )
target_link_libraries( securebuffer_test
    ksecretservicebackend
    Qt5::Test
)
add_test( SecureBufferTest securebuffer_test )
