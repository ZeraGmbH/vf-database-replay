cmake_minimum_required(VERSION 3.1)

#set project name and language
project(vf-database-replay LANGUAGES CXX)

#Automoc and RCC ON. Needed in QT projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#Set project Version
set(VfReplay_VERSION_MAJOR "0")
set(VfReplay_VERSION_MINOR "0")
set(VfReplay_VERSION_PATCH "1")

set(PROJECT_VERSION "${VfReplay_VERSION_MAJOR}.${VfReplay_VERSION_MINOR}.${VfReplay_VERSION_PATCH}")

#Adding cmake functions like feature_summary()s
include(FeatureSummary)
include(GNUInstallDirs)

#Find dependecies
find_package(Qt5 COMPONENTS Core Sql Network CONFIG REQUIRED)
find_package(VfEvent REQUIRED)
find_package(VfHelpers REQUIRED)
find_package(VfComponent REQUIRED)
find_package(VfStorageHash REQUIRED)
find_package(VfQml REQUIRED)
find_package(VfNet2 REQUIRED)
find_package(VfLogger REQUIRED)

#sum up project Files 
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

#create executable
add_executable(VfReplay 
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )


#link dependencies
target_link_libraries(VfReplay 
    PRIVATE
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    VeinMeta::VfEvent
    VeinMeta::VfHelpers
    VeinMeta::VfComponent
    VeinMeta::VfStorageHash
    VeinMeta::VfNet2
    VeinMeta::VfLogger
    )	


# announce headers - target perspective
target_include_directories(VfReplay 
    PUBLIC
    $<BUILD_INTERFACE:${MY_LIBRARY_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/VfReplay>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )


#install VfReplay to bin 
install(TARGETS VfReplay
    EXPORT VfReplayExport
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )


# spawn out some info on configuration
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)






