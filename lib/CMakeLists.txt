include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)


SET( daemonlib_SRCS
    jobqueue.cpp
    queuedjob.cpp
    secrettool.cpp
)

KDE4_ADD_LIBRARY( daemonlib STATIC ${daemonlib_SRCS} )

target_link_libraries( daemonlib
    ${KDE4_KDECORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
)
