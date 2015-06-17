project(pam_ksecrets)
cmake_minimum_required(VERSION 3.2.2)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )

find_package(PAM REQUIRED)
find_package(LibGcrypt 1.6.0 REQUIRED)
include(GNUInstallDirs)

INCLUDE_DIRECTORIES(
  ${PAM_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../ksecrets-crypt
)

set(pam_ksecret_SRC
  pam_ksecrets.c)

add_library(pam_ksecrets SHARED ${pam_ksecret_SRC})
set_target_properties(pam_ksecrets PROPERTIES PREFIX "")
target_link_libraries(pam_ksecrets
  ksecrets-crypt
  ${LIBGCRYPT_LIBRARIES}
  ${PAM_LIBRARIES}
  keyutils)

install(TARGETS pam_ksecrets DESTINATION /${CMAKE_INSTALL_LIBDIR}/security)

