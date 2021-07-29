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

target_include_directories(replay-bot PRIVATE
  libraries/gd.h/include
  libraries/gd.h
  libraries/minhook/include
  libraries/nfd/src/include
  libraries/cocos-headers/cocos2dx/
  libraries/cocos-headers/cocos2dx/include
  libraries/cocos-headers/cocos2dx/kazmath/include
  libraries/cocos-headers/cocos2dx/platform/win32
  libraries/cocos-headers/cocos2dx/platform/third_party/win32
  libraries/cocos-headers/cocos2dx/platform/third_party/win32/OGLES
  libraries/cocos-headers/extensions/
)

add_subdirectory(libraries/minhook)

target_link_libraries(replay-bot
  minhook
  ${CMAKE_SOURCE_DIR}/libraries/cocos-headers/cocos2dx/libcocos2d.lib
  ${CMAKE_SOURCE_DIR}/libraries/cocos-headers/extensions/libExtensions.lib
)
