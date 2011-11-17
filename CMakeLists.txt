cmake_minimum_required(VERSION 2.6.4)

PROJECT (ksecrets)

FIND_PACKAGE (KDE4 REQUIRED)
FIND_PACKAGE (QCA2 REQUIRED)

ADD_DEFINITIONS (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include(MacroLibrary)
include(KDE4Defaults)
include(MacroLogFeature)

add_subdirectory(ksecretsservice)
add_subdirectory(ksecrets)
add_subdirectory(kwl2kss)
add_subdirectory(kio)
add_subdirectory(secretsync)

#include(FeatureSummary)

macro_display_feature_log()
