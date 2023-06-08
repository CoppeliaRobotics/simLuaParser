cmake_minimum_required(VERSION 2.8.11)
project(simLuaParser)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Boost REQUIRED)

if(NOT COPPELIASIM_INCLUDE_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(COPPELIASIM_INCLUDE_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/include)
    else()
        set(COPPELIASIM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    endif()
endif()
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${COPPELIASIM_INCLUDE_DIR}/cmake)
find_package(CoppeliaSim 4.5.1.7 REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simLuaParser.lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    plugin.cpp
    tinyxml2.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simStack/stackObject.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simStack/stackNull.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simStack/stackBool.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simStack/stackNumber.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simStack/stackString.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simStack/stackArray.cpp
    ${COPPELIASIM_INCLUDE_DIR}/simStack/stackMap.cpp
)

coppeliasim_add_plugin(simLuaParser SOURCES ${SOURCES})
target_link_libraries(simLuaParser Boost::boost)
