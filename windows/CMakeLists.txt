cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "keyboard_event")
project(${PROJECT_NAME} LANGUAGES CXX)

message(VERBOSE "CMAKE_CXX_STANDARD= " ${CMAKE_CXX_STANDARD})
# if(NOT CMAKE_CXX_STANDARD)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# endif()

# This value is used when generating builds using this plugin, so it must not be
# changed
set(PLUGIN_NAME "keyboard_event_plugin")

# ---------------------------------------------------------------------------------------
# uncomment next line to debug keyboard_event with spdlog
# ---------------------------------------------------------------------------------------
# set(KEYEVENT_DEBUG ON)
if(DEFINED ENV{KEYEVENT_DEBUG})
  set(KEYEVENT_DEBUG ON)
endif()
message(STATUS "keyboard_event KEYEVENT_DEBUG=" ${KEYEVENT_DEBUG})
if(KEYEVENT_DEBUG)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D KEYEVENT_DEBUG")
  if(NOT EXISTS
     "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/spdlog/CMakeLists.txt")
    message(
      STATUS
        "The submodules were not downloaded! Trying to update submodules ...")
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
      # Update submodules as needed
      option(GIT_SUBMODULE "Check submodules during build" ON)
      if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(
          COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
          message(
            FATAL_ERROR
              "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
          )
        endif()
      endif()
    else()
      message(
        FATAL_ERROR
          "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again."
      )
    endif()
  endif()
  add_subdirectory("../third_party/spdlog" "../third_party/bin")
  include_directories("../third_party/spdlog/include")
endif()

add_library(
  ${PLUGIN_NAME} SHARED "map_serializer.cpp" "keyboard_event_plugin.cpp"
                        "map_serializer.h" "codeconvert.cpp" "timestamp.h")
set(TIMESTAMP_H timestamp.h)
add_custom_target(
  ${TIMESTAMP_H}
  COMMAND cmd /C echo ${PROJECT_SOURCE_DIR}
  COMMAND cmd /C timestamp.bat .
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
add_dependencies(${PLUGIN_NAME} ${TIMESTAMP_H})

target_compile_options(${PLUGIN_NAME} PRIVATE /utf-8 /std:c++latest /bigobj)
add_definitions(-D_HAS_EXCEPTIONS=1 # error LNK2019: std::bad_cast
)

apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories(${PLUGIN_NAME}
                           INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(KEYEVENT_DEBUG)
  target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin
                                               spdlog::spdlog)
else()
  target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)
endif()

# List of absolute paths to libraries that should be bundled with the plugin
set(keyboard_event_bundled_libraries
    ""
    PARENT_SCOPE)
