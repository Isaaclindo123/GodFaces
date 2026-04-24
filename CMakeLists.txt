cmake_minimum_required(VERSION 3.21)

project(GodFace VERSION 1.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "P4RRA, SE ESSE ERRO ACONTECER EU QUERO QUE SE DANE ATÉ O MEU PI-")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(GodFace SHARED ${SOURCES})

setup_geode_mod(GodFace)

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources")
    file(GLOB_RECURSE RESOURCES resources/*)
    target_sources(GodFace PRIVATE ${RESOURCES})
endif()


