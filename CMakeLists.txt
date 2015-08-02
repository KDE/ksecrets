cmake_minimum_required(VERSION 2.8.12)

project(KSecrets)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(FeatureSummary)
find_package(ECM 5.11.0 NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED
  Core
  Gui
  Widgets
  Concurrent
  DBus
  Test)

option(BUILD_TESTS "Build KSecrets with unit tests" ON)

set(KF5_VERSION "5.12.0")
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

if(NOT kdelibs_SOURCE_DIR)
    find_package(KF5 ${KF5_VERSION} REQUIRED
        CoreAddons
        WidgetsAddons
        Service
        Config
        WindowSystem
        I18n
    )
endif()


add_subdirectory(src)

if(BUILD_TESTS)
   add_subdirectory(autotests)
endif()

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
