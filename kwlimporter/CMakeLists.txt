include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/kwalletbackend
                    ${CMAKE_CURRENT_SOURCE_DIR}/kwalletbackend
)

########### Configure checks for kwalletbackend ###############

check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(sys/bitypes.h HAVE_SYS_BITYPES_H)

configure_file (kwalletbackend/config-kwalletbackend.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/kwalletbackend/config-kwalletbackend.h )

########### kwl_importer ###############

set (kwl_importer_SRCS
    importsinglewalletjob.cpp
    kwlimporterjob.cpp
    kwalletbackend/blockcipher.cc
    kwalletbackend/blowfish.cc
    kwalletbackend/cbc.cc
    kwalletbackend/sha1.cc
    kwalletbackend/kwalletentry.cc
    kwalletbackend/kwalletbackend.cc
)

kss_add_dbus_interface(kwl_importer_SRCS ../../client/org.freedesktop.Secret.Collection.xml collection_interface)
kss_add_dbus_interface(kwl_importer_SRCS ../../client/org.freedesktop.Secret.Item.xml item_interface )
kss_add_dbus_interface(kwl_importer_SRCS ../../client/org.freedesktop.Secret.Prompt.xml prompt_interface )
kss_add_dbus_interface(kwl_importer_SRCS ../../client/org.freedesktop.Secret.Service.xml service_interface )
kss_add_dbus_interface(kwl_importer_SRCS ../../client/org.freedesktop.Secret.Session.xml session_interface )

kde4_add_library(kwl_importer ${kwl_importer_SRCS})
target_link_libraries(kwl_importer ${KDE4_KDECORE_LIBRARY} ${QT_QTDBUS_LIBRARY} 
    ksecretservicefrontend 
    ksecretservicebackend
    ksecretserviceclient
)

# link with advapi32 on windows
if(WIN32 AND NOT WINCE)
   target_link_libraries(kwl_importer advapi32)
endif(WIN32 AND NOT WINCE)
