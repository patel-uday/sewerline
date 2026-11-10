cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(SEWERLINE VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
foreach(subsystem RENDER GPU JOYSTICK HAPTIC SENSOR CAMERA POWER DIALOG TRAY HIDAPI VULKAN OPENGLES)
  set(SDL_${subsystem} OFF CACHE BOOL "" FORCE)
endforeach()
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SDL
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.2.26
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(SDL)

# This fixed, statically linked build cannot hot-swap SDL at runtime.
# SDL requires editing its internal header to disable dynapi. Turning it off
# lets /OPT:REF discard unused API entry points and their implementation.
set(dynapi_header "${sdl_SOURCE_DIR}/src/dynapi/SDL_dynapi.h")
file(READ "${dynapi_header}" dynapi_source)
string(REPLACE "#define SDL_DYNAMIC_API 1" "#define SDL_DYNAMIC_API 0 /* SEWERLINE: fixed static build; modified from upstream. */" dynapi_small "${dynapi_source}")
if(NOT dynapi_source STREQUAL dynapi_small)
  file(WRITE "${dynapi_header}" "${dynapi_small}")
endif()

enable_language(RC)
find_program(ASSET_POWERSHELL NAMES pwsh powershell REQUIRED)
execute_process(COMMAND "${ASSET_POWERSHELL}" -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare-assets.ps1" -OutputDir "${CMAKE_CURRENT_BINARY_DIR}/packed-assets" COMMAND_ERROR_IS_FATAL ANY)
set(asset_rc "")
set(asset_index "#pragma once\n#include <string_view>\ninline int assetId(std::string_view path){\n")
set(asset_id 100)
file(GLOB_RECURSE runtime_assets CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" assets/*.png assets/*.wav)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${runtime_assets} tools/prepare-assets.ps1)
foreach(asset IN LISTS runtime_assets)
  get_filename_component(asset_name "${asset}" NAME)
  string(APPEND asset_rc "${asset_id} RCDATA \"${CMAKE_CURRENT_BINARY_DIR}/packed-assets/${asset_name}.bin\"\n")
  string(APPEND asset_index "if(path == \"${asset}\") return ${asset_id};\n")
  math(EXPR asset_id "${asset_id} + 1")
endforeach()
string(APPEND asset_index "return 0;\n}\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/assets.rc" "${asset_rc}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/AssetIndex.hpp" "${asset_index}")

add_library(sewerline_core
  src/Maze.cpp
  src/Player.cpp)
target_include_directories(sewerline_core PUBLIC src)

add_executable(sewerline WIN32
  src/main.cpp
  src/Game.cpp
  src/Renderer.cpp
  src/Audio.cpp)
target_sources(sewerline PRIVATE src/Assets.cpp "${CMAKE_CURRENT_BINARY_DIR}/assets.rc")
target_include_directories(sewerline PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(sewerline PRIVATE sewerline_core SDL3::SDL3-static opengl32 windowscodecs ole32 cabinet)
target_compile_definitions(sewerline PRIVATE SDL_MAIN_USE_CALLBACKS)
if(MSVC)
  target_compile_options(SDL3-static PRIVATE $<$<CONFIG:Release>:/O1> $<$<CONFIG:Release>:/Gy> $<$<CONFIG:Release>:/Gw>)
  set_property(TARGET sewerline sewerline_core SDL3-static PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  target_compile_options(sewerline PRIVATE $<$<CONFIG:Release>:/O1> $<$<CONFIG:Release>:/Gy> $<$<CONFIG:Release>:/Gw>)
  target_compile_options(sewerline_core PRIVATE $<$<CONFIG:Release>:/O1> $<$<CONFIG:Release>:/Gy> $<$<CONFIG:Release>:/Gw>)
  target_link_options(sewerline PRIVATE $<$<CONFIG:Release>:/OPT:REF> $<$<CONFIG:Release>:/OPT:ICF> $<$<CONFIG:Release>:/INCREMENTAL:NO>)
  target_compile_options(sewerline PRIVATE /W4 /permissive-)
  target_compile_options(sewerline_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(sewerline PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(sewerline_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(sewerline_tests EXCLUDE_FROM_ALL tests/tests.cpp)
target_link_libraries(sewerline_tests PRIVATE sewerline_core)
enable_testing()
add_test(NAME sewerline_logic COMMAND sewerline_tests)

install(TARGETS sewerline RUNTIME DESTINATION .)
install(FILES README.md LICENSE.txt assets/audio/LICENSES.md DESTINATION .)
install(FILES "${sdl_SOURCE_DIR}/LICENSE.txt" DESTINATION . RENAME SDL-LICENSE.txt)
