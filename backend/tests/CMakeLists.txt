INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR})

KDE4_ADD_EXECUTABLE (temporary_collection_test temporarytest.cpp)
TARGET_LINK_LIBRARIES (temporary_collection_test
    ksecretservicebackend
    ksecretserviceui
    daemonlib
    ${QT_QTTEST_LIBRARIES}
)

ADD_TEST (TemporaryCollectionTest temporary_collection_test)

KDE4_ADD_EXECUTABLE (ksecret_collection_test ksecrettest.cpp)
TARGET_LINK_LIBRARIES (ksecret_collection_test
    ksecretservicebackend
    ksecretserviceui
    daemonlib
    ${QT_QTTEST_LIBRARIES}
)

ADD_TEST (KSecretCollectionTest ksecret_collection_test)

KDE4_ADD_EXECUTABLE (ksecret_collection_ui_test ksecretuitest.cpp)
TARGET_LINK_LIBRARIES (ksecret_collection_ui_test
    ksecretservicebackend
    ksecretserviceui
    daemonlib
    ${QT_QTTEST_LIBRARIES}
)

ADD_TEST (KSecretCollectionUiTest ksecret_collection_ui_test)

KDE4_ADD_EXECUTABLE (securebuffer_test securebuffertest.cpp)
TARGET_LINK_LIBRARIES (securebuffer_test ksecretservicebackend ${QT_QTTEST_LIBRARIES})

ADD_TEST (SecureBufferTest securebuffer_test)
