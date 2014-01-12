#
# This is a command-line utility for secrets handling
#
PROJECT (ksecrets)

include(MacroLibrary)
include(KDE4Defaults)

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)


#
# ksecrets
# 

KDE4_ADD_EXECUTABLE (ksecrets
    main.cpp
    ksecretsapp.cpp
    ksecretsappjob.cpp
)

TARGET_LINK_LIBRARIES (ksecrets
    ksecretsservice
    ${KDE4_KDEUI_LIBS}
)

install( TARGETS ksecrets ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ksecrets.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )

