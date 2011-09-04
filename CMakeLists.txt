project(kwl2kss)


include(MacroLibrary)
include(KDE4Defaults)
include(PkgConfigGetVar)

########### Configure checks for kwalletbackend ###############

check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(sys/bitypes.h HAVE_SYS_BITYPES_H)

configure_file (kwalletbackend/config-kwalletbackend.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/kwalletbackend/config-kwalletbackend.h )


########### kwl2kss ###############

INCLUDE_DIRECTORIES (
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/kwalletbackend/
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=285)

#
# kwl2kss
# 

KDE4_ADD_EXECUTABLE (kwl2kss 
    main.cpp
    kwlimporterjob.cpp
    importsinglewalletjob.cpp
    kwalletbackend/blockcipher.cc
    kwalletbackend/blowfish.cc
    kwalletbackend/cbc.cc
    kwalletbackend/kwalletbackend.cc
    kwalletbackend/kwalletentry.cc
    kwalletbackend/sha1.cc
)

TARGET_LINK_LIBRARIES (kwl2kss
    ${KDE4_KDEUI_LIBS}
)


install( TARGETS kwl2kss ${INSTALL_TARGETS_DEFAULT_ARGS})

# Does this litte utility evert need a .desktop file? I don't think so.
#install( FILES kwl2kss.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


# link with advapi32 on windows
if(WIN32 AND NOT WINCE)
   target_link_libraries(kwl_importer advapi32)
endif(WIN32 AND NOT WINCE)

