project(ksecrets-crypt)
cmake_minimum_required(VERSION 3.2.2)

set(ksecrets-crypt_SRC
  passwords.c)

add_library(ksecrets-crypt STATIC ${ksecrets-crypt_SRC})
set_target_properties(ksecrets-crypt PROPERTIES PREFIX "")
# target_link_libraries(ksecrets-crypt )

