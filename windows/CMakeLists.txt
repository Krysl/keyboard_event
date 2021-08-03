cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "keyboard_event")
project(${PROJECT_NAME} LANGUAGES CXX)

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
if(DEFINED ENV{KEYEVENT_DEBUG_NONGIT})
  set(KEYEVENT_DEBUG_NONGIT ON)
endif()
message(STATUS "keyboard_event KEYEVENT_DEBUG=" ${KEYEVENT_DEBUG})
if(KEYEVENT_DEBUG)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D KEYEVENT_DEBUG")

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
    set(KEYEVENT_DEBUG_NONGIT OFF)
  endif()
  if(KEYEVENT_DEBUG_NONGIT)
    # this is in git
    if(NOT EXISTS
       "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/spdlog/CMakeLists.txt")
      # if not download spdlog
      message(
        STATUS
          "The submodules were not downloaded! Trying to update submodules ...")
      find_package(Git QUIET)
      if(GIT_FOUND)
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
  else()
    # this is not git, maybe user is using pub
    set(SPDLOG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/spdlog")
    set(SPDLOG_BUILD_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/build/spdlog")
    set(SPDLOG_INSTALL_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/bin/spdlog")
    include(ExternalProject)
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                            ${SPDLOG_BUILD_DIR})
    ExternalProject_Add(
      spdlog
      TMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/tmp/spdlog"
      STAMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/stamp/spdlog"
      BINARY_DIR ${SPDLOG_BUILD_DIR}
      SOURCE_DIR ${SPDLOG_DIR}
      INSTALL_DIR ${SPDLOG_INSTALL_DIR}
      GIT_REPOSITORY git@github.com:gabime/spdlog.git
      GIT_TAG v1.9.1
      PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/spdlog"
      CONFIGURE_COMMAND cmake -S ../../spdlog -B .
      BUILD_COMMAND cmake --build . --config
                    "$<IF:$<CONFIG:Debug>,Debug,Release>"
      INSTALL_COMMAND
        cmake --install . --prefix ${SPDLOG_INSTALL_DIR} --config
        "$<IF:$<CONFIG:Debug>,Debug,Release>" WORKING_DIRECTORY ${SPDLOG_DIR}
      LOG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/log/spdlog"
      LOG_CONFIGURE ON
      LOG_BUILD ON
      LOG_INSTALL ON
      LOG_MERGED_STDOUTERR ON)
    find_package(
      spdlog QUIET PATHS
      "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/bin/spdlog/lib/cmake/spdlog"
      NO_DEFAULT_PATH)
    include_directories("../third_party/bin/spdlog/include")
  endif()
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
add_definitions(-D_HAS_EXCEPTIONS=1) # error LNK2019: std::bad_cast

apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories(${PLUGIN_NAME}
                           INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(KEYEVENT_DEBUG)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/bin/spdlog/lib")
    target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin
                                                 spdlog::spdlog)
  else()
    target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)
  endif()
else()
  target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)
endif()

# List of absolute paths to libraries that should be bundled with the plugin
set(keyboard_event_bundled_libraries
    ""
    PARENT_SCOPE)
