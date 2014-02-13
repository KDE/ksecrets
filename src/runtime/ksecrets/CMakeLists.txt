#
# This is a command-line utility for secrets handling
#
project( ksecrets )

include_directories(
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../api/ksecrets
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)


#
# ksecrets
# 

add_executable(ksecrets
    main.cpp
    ksecretsapp.cpp
    ksecretsappjob.cpp
)

target_link_libraries( ksecrets
    KF5::Secrets
    KF5::CoreAddons
    KF5::I18n
    Qt5::Core
)

install( TARGETS ksecrets ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ksecrets.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )

