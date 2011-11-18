
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR})

ADD_SUBDIRECTORY (tests)

SET (ksecretservice_ui_SRCS
    abstractuijobs.cpp
    abstractacljobs.cpp
    dialogaskaclprefsjob.cpp
    nouiaskaclprefsjob.cpp
    abstractacljobs.cpp
    dialoguimanager.cpp
    nouimanager.cpp
    dialogaskcreateaclprefs.cpp
    dialogaskunlockaclprefs.cpp
    ../peer.cpp
)

KDE4_ADD_UI_FILES( ksecretservice_ui_SRCS
    dialogaskcreateaclprefs.ui
    dialogaskunlockaclprefs.ui
)
    
KDE4_ADD_LIBRARY (ksecretserviceui STATIC ${ksecretservice_ui_SRCS})

TARGET_LINK_LIBRARIES (
    ksecretserviceui 
    daemonlib
    ${KDE4_KDEUI_LIBS} 
    ${QCA2_LIBRARIES}
)
