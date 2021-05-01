cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(replay-bot)

find_file(WINDOWS_HEADER windows.h)
if(NOT WINDOWS_HEADER)
  message(FATAL_ERROR "Can't find windows.h!")
endif()

file(
  GLOB_RECURSE SOURCE_FILES
  src/*.cpp
  libraries/nfd/src/nfd_common.c
  libraries/nfd/src/nfd_win.cpp
)
add_library(replay-bot SHARED ${SOURCE_FILES})

set(CAPPUCCINO_DIR ${CMAKE_SOURCE_DIR}/libraries/cappuccino-sdk/template/default-cpp/cappuccino-sdk)

target_include_directories(replay-bot PRIVATE
  libraries/gd.h/include
  libraries/gd.h
  libraries/minhook/include
  libraries/nfd/src/include
  ${CAPPUCCINO_DIR}/incl/cocos2dx/
  ${CAPPUCCINO_DIR}/incl/cocos2dx/include
  ${CAPPUCCINO_DIR}/incl/cocos2dx/kazmath/include
  ${CAPPUCCINO_DIR}/incl/cocos2dx/platform/win32
  ${CAPPUCCINO_DIR}/incl/cocos2dx/platform/third_party/win32
  ${CAPPUCCINO_DIR}/incl/cocos2dx/platform/third_party/win32/OGLES
)

add_subdirectory(libraries/minhook)
# TODO: make it not build minhook every time
target_link_libraries(replay-bot minhook)

target_link_libraries(replay-bot ${CAPPUCCINO_DIR}/lib/cappuccino.lib)