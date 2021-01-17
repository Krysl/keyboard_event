cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "keyboard_event")
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_PREFIX_PATH "D:/Program Files/CMake")
# set(CMAKE_MODULE_PATH "D:/Program Files/CMake")
# MESSAGE("CMAKE_PREFIX_PATH= ${CMAKE_PREFIX_PATH}")
# MESSAGE("CMAKE_MODULE_PATH= ${CMAKE_MODULE_PATH}")

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "keyboard_event_plugin")

find_package(spdlog REQUIRED)

add_library(${PLUGIN_NAME} SHARED
  "keyboard_event_plugin.cpp"
  "keyboard_event.cpp"
)
apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin spdlog::spdlog)

# List of absolute paths to libraries that should be bundled with the plugin
set(keyboard_event_bundled_libraries
  ""
  PARENT_SCOPE
)
