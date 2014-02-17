include_directories(
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

#add_subdirectory( tests )

set( ksecretservice_ui_SRCS
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

qt5_wrap_ui( ksecretservice_ui_SRCS
    dialogaskcreateaclprefs.ui
    dialogaskunlockaclprefs.ui
)

add_library( ksecretserviceui STATIC ${ksecretservice_ui_SRCS} )

target_link_libraries(
    ksecretserviceui 
    daemonlib
    KF5::CoreAddons
    KF5::WidgetsAddons
    KF5::I18n
    Qt5::Core
    Qt5::Widgets
    ${QCA2_LIBRARIES}
)
