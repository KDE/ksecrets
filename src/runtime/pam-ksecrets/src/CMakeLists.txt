find_package(PAM REQUIRED)
include(GNUInstallDirs)

INCLUDE_DIRECTORIES(
  ${PAM_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(pam_ksecret_SRC
  pam_ksecrets.c)

add_library(pam_ksecrets SHARED ${pam_ksecret_SRC})
set_target_properties(pam_ksecrets PROPERTIES PREFIX "")
target_link_libraries(pam_ksecrets
  ksecretslib
  ${PAM_LIBRARIES})

install(TARGETS pam_ksecrets DESTINATION /${CMAKE_INSTALL_LIBDIR}/security)

