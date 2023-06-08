cmake_minimum_required(VERSION 2.8.11)
project(simLuaParser)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Boost REQUIRED)

if(NOT LIBPLUGIN_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(LIBPLUGIN_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/libPlugin)
    else()
        set(LIBPLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libPlugin)
    endif()
endif()
list(APPEND CMAKE_MODULE_PATH ${LIBPLUGIN_DIR}/cmake)
find_package(CoppeliaSim 4.5.1.7 REQUIRED)

include_directories(${COPPELIASIM_INCLUDE_DIR}/stack)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simLuaParser.lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    plugin.cpp
    tinyxml2.cpp
    ${COPPELIASIM_COMMON_DIR}/stack/stackObject.cpp
    ${COPPELIASIM_COMMON_DIR}/stack/stackNull.cpp
    ${COPPELIASIM_COMMON_DIR}/stack/stackBool.cpp
    ${COPPELIASIM_COMMON_DIR}/stack/stackNumber.cpp
    ${COPPELIASIM_COMMON_DIR}/stack/stackString.cpp
    ${COPPELIASIM_COMMON_DIR}/stack/stackArray.cpp
    ${COPPELIASIM_COMMON_DIR}/stack/stackMap.cpp
)

coppeliasim_add_plugin(simLuaParser SOURCES ${SOURCES})
target_link_libraries(simLuaParser Boost::boost)
