cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "keyboard_event")
project(${PROJECT_NAME} LANGUAGES CXX)

# MESSAGE("CMAKE_CXX_STANDARD= " ${CMAKE_CXX_STANDARD})

# if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
#     set(CMAKE_CXX_STANDARD_REQUIRED ON)
# endif()

add_definitions(
  -D_HAS_EXCEPTIONS=1  # error LNK2019: std::bad_cast
)

add_compile_options(/utf-8 /std:c++latest /bigobj)
# set(ENV{CMAKE_PREFIX_PATH} "D:/Program Files/CMake")
# set(ENV{CMAKE_MODULE_PATH} "D:/Program Files/CMake")
# MESSAGE("CMAKE_PREFIX_PATH= " $ENV{CMAKE_PREFIX_PATH})
# MESSAGE("CMAKE_MODULE_PATH= " $ENV{CMAKE_MODULE_PATH})

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "keyboard_event_plugin")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/spdlog/CMakeLists.txt")
  message(STATUS "The submodules were not downloaded! Trying to update submodules ...")
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
    # Update submodules as needed
        option(GIT_SUBMODULE "Check submodules during build" ON)
        if(GIT_SUBMODULE)
            message(STATUS "Submodule update")
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                            RESULT_VARIABLE GIT_SUBMOD_RESULT)
            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
            endif()
        endif()
    else()
        message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
    endif()
endif()

add_subdirectory("../third_party/spdlog" "../third_party/bin")
include_directories("../third_party/spdlog/include")

add_library(${PLUGIN_NAME} SHARED
  "map_serializer.cpp"
  "keyboard_event_plugin.cpp"
  "map_serializer.h"
  "codeconvert.cpp"
  "timestamp.h"
)
set(TIMESTAMP_H timestamp.h)
add_custom_target(${TIMESTAMP_H}
  COMMAND cmd /C echo ${PROJECT_SOURCE_DIR}
  COMMAND cmd /C timestamp.bat .
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_dependencies(${PLUGIN_NAME}
  ${TIMESTAMP_H}
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
