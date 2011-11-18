INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR})

# involves dialogs, thus not an automatic test
KDE4_ADD_EXECUTABLE (dialoguimanager_test dialoguimanagertest.cpp)
TARGET_LINK_LIBRARIES (dialoguimanager_test
    ksecretserviceui
    daemonlib
    ${QT_QTTEST_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
)

KDE4_ADD_EXECUTABLE (nouimanager_test nouimanagertest.cpp)
TARGET_LINK_LIBRARIES (nouimanager_test
    ksecretserviceui
    daemonlib
    ${QT_QTTEST_LIBRARIES}
)

KDE4_ADD_EXECUTABLE (dialogaskaclprefs_test dialogaskaclprefstest.cpp )
TARGET_LINK_LIBRARIES( dialogaskaclprefs_test
    ksecretserviceui
    daemonlib
    ${QT_QTTEST_LIBRARIES}
)

ADD_TEST (NoUiManagerTest nouimanager_test)
#ADD_TEST (DialogUiManagerTest dialoguimanager_test)
#ADD_TEST (DialogAskAclPrefsTest dialogaskaclprefs_test )
