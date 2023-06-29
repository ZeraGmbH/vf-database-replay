cmake_minimum_required(VERSION 3.1)

project(vf-database-replay LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(VfReplay_VERSION_MAJOR "0")
set(VfReplay_VERSION_MINOR "0")
set(VfReplay_VERSION_PATCH "1")
set(PROJECT_VERSION "${VfReplay_VERSION_MAJOR}.${VfReplay_VERSION_MINOR}.${VfReplay_VERSION_PATCH}")

include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt5 COMPONENTS Core Sql Network CONFIG REQUIRED)
find_package(VfLogger REQUIRED)

file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    )

file(GLOB RESOURCES
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

add_executable(VfReplay 
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )

target_link_libraries(VfReplay 
    PRIVATE
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    Zera::VfEvent
    Zera::VfHelpers
    Zera::VfComponent
    Zera::VfStorageHash
    Zera::VfNet2
    VeinMeta::VfLogger
    )

target_include_directories(VfReplay 
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

install(TARGETS VfReplay
    EXPORT VfReplayExport
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
