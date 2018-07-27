cmake_minimum_required(VERSION 2.8.11)
project(v_repExtLuaParser)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

find_package(Boost REQUIRED)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoppeliaRobotics-modules)
find_package(VREP 3.5.0 REQUIRED)

include_directories(${VREP_INCLUDE})
include_directories(${VREP_INCLUDE}/stack)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${Boost_INCLUDE_DIRS})

vrep_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simExtLuaParser.lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    plugin.cpp
    tinyxml2.cpp
    ${VREP_COMMON}/stack/stackObject.cpp
    ${VREP_COMMON}/stack/stackNull.cpp
    ${VREP_COMMON}/stack/stackBool.cpp
    ${VREP_COMMON}/stack/stackNumber.cpp
    ${VREP_COMMON}/stack/stackString.cpp
    ${VREP_COMMON}/stack/stackArray.cpp
    ${VREP_COMMON}/stack/stackMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/external/v_repPlusPlus/Plugin.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/generated/stubs.cpp
    ${VREP_EXPORTED_SOURCES}
)

set(LIBRARIES
    ${Boost_LIBRARIES}
    ${VREP_LIBRARIES}
)

add_library(v_repExtLuaParser SHARED ${SOURCES})
target_link_libraries(v_repExtLuaParser ${LIBRARIES})

