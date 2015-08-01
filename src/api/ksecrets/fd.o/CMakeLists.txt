set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Collection.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Item.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Prompt.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Service.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Session.xml
    PROPERTIES INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/ksecretsservicedbustypes.h )

qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Collection.xml collection_interface)
qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Item.xml item_interface)
qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Prompt.xml prompt_interface)
qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Service.xml service_interface)
qt5_add_dbus_interface(ksecrets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.Secret.Session.xml session_interface)


INSTALL(FILES
    ${ksecretsservice_xml}
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
