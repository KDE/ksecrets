include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# involves dialogs, thus not an automatic test
add_executable( dialoguimanager_test dialoguimanagertest.cpp )
target_link_libraries( dialoguimanager_test
    ksecretserviceui
    daemonlib
    KF5::CoreAddons
    Qt5::Test
)

add_executable( nouimanager_test nouimanagertest.cpp )
target_link_libraries( nouimanager_test
    ksecretserviceui
    daemonlib
    Qt5::Test
)

add_executable( dialogaskaclprefs_test dialogaskaclprefstest.cpp )
target_link_libraries( dialogaskaclprefs_test
    ksecretserviceui
    daemonlib
    Qt5::Test
)

add_test( NoUiManagerTest nouimanager_test )
#ADD_TEST (DialogUiManagerTest dialoguimanager_test)
#ADD_TEST (DialogAskAclPrefsTest dialogaskaclprefs_test )
