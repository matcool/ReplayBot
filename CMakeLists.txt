cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(libraries/mat-dash/include.cmake)

project(replay-bot)

if (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
  add_compile_definitions(SHOW_CONSOLE)
endif()

add_compile_definitions(UNICODE _UNICODE)

file(
  GLOB_RECURSE SOURCE_FILES
  src/*.cpp
  libraries/nfd/src/nfd_common.c
  libraries/nfd/src/nfd_win.cpp
)
add_library(replay-bot SHARED ${SOURCE_FILES})

# https://stackoverflow.com/a/63924044/9124836
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file("${CMAKE_SOURCE_DIR}/src/version.hpp.in"
               "${CMAKE_SOURCE_DIR}/src/version.hpp")

target_include_directories(replay-bot PRIVATE
  libraries/gd.h/include
  libraries/gd.h
  libraries/minhook/include
  libraries/nfd/src/include
)

# this is so stupid
set(CMAKE_SIZEOF_VOID_P 4)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  # ensure 32 bit on clang
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target i386-pc-windows-msvc ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target i386-pc-windows-msvc ")
else()
  target_compile_options(replay-bot PRIVATE /permissive- /W4 /w14640)
endif()


add_subdirectory(libraries/minhook)
add_subdirectory(libraries/cocos-headers)

target_link_libraries(replay-bot
  minhook
  cocos2d
  opengl32
  ${CMAKE_SOURCE_DIR}/libraries/glew32.lib
)
