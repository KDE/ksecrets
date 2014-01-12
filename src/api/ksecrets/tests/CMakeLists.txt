
include(ECMMarkAsTest)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(ksecrets_api_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname} Qt5::Test KF5::Secrets)
  endforeach(_testname)
endmacro()

ksecrets_api_tests(
    ksecretsservice-test
)

