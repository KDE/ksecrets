PROJECT (ksecretserviced)

FIND_PACKAGE (QCA2 REQUIRED)

include(MacroLibrary)
include(KDE4Defaults)

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${QCA2_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

ADD_SUBDIRECTORY (lib)
ADD_SUBDIRECTORY (ui)
ADD_SUBDIRECTORY (backend)
ADD_SUBDIRECTORY (frontend)
ADD_SUBDIRECTORY (kwlimporter)

#
# ksecretserviced
# 

KDE4_ADD_EXECUTABLE (ksecretserviced main.cpp)

TARGET_LINK_LIBRARIES (ksecretserviced
   ksecretservicebackend
   ksecretservicefrontend
   ksecretservicelib
   ksecretserviceui
   ${KDE4_KDEUI_LIBS}
)


install( TARGETS ksecretserviced ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ksecretserviced.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
