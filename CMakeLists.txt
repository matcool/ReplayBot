cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(replay-bot VERSION 1.0.0)

find_file(WINDOWS_HEADER windows.h)
if(NOT WINDOWS_HEADER)
  message(FATAL_ERROR "Can't find windows.h!")
endif()

file(
  GLOB_RECURSE SOURCE_FILES
  src/*.cpp
  # nice
  libraries/cappuccino-sdk/template/default-cpp/cappuccino-sdk/incl/cocos2dx/custom/Sprites/CCMenuItemSpriteExtra/CCMenuItemSpriteExtra.cpp
)
add_library(replay-bot SHARED ${SOURCE_FILES})

target_include_directories(replay-bot PRIVATE
  libraries/minhook/include
  libraries/cappuccino-sdk/template/default-cpp/cappuccino-sdk/incl/cocos2dx/
  libraries/cappuccino-sdk/template/default-cpp/cappuccino-sdk/incl/cocos2dx/include
  libraries/cappuccino-sdk/template/default-cpp/cappuccino-sdk/incl/cocos2dx/kazmath/include
  libraries/cappuccino-sdk/template/default-cpp/cappuccino-sdk/incl/cocos2dx/platform/win32
  libraries/cappuccino-sdk/template/default-cpp/cappuccino-sdk/incl/cocos2dx/platform/third_party/win32
  libraries/cappuccino-sdk/template/default-cpp/cappuccino-sdk/incl/cocos2dx/platform/third_party/win32/OGLES
)

add_subdirectory(libraries/minhook)
# TODO: make it not build minhook every time
target_link_libraries(replay-bot minhook)

target_link_libraries(replay-bot ${CMAKE_SOURCE_DIR}/libraries/cappuccino-sdk/template/default-cpp/cappuccino-sdk/lib/cappuccino.lib)

set_target_properties(replay-bot PROPERTIES PREFIX "")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})