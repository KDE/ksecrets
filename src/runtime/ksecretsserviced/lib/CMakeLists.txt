include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)


SET( daemonlib_SRCS
    secrettool.cpp
    daemonjob.cpp
)

add_library( daemonlib STATIC ${daemonlib_SRCS} )

target_link_libraries( daemonlib
    KF5::CoreAddons
    Qt5::DBus
)
