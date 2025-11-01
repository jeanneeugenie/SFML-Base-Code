# CMakeList.txt : CMake project for STDISICM_HO2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# (Removed malformed global runtime lines)
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug$<$<CONFIG:Debug>:Debug>")
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Project Name
project ("SFML")

# Change the output directory of the exe (zip the bin if needed to submit)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set base path for 3rd party references projects
include(FetchContent)
# Set directory of 3rd party libraries
set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty CACHE PATH "Base Directory of libs" FORCE)
# Set directory of assets
set(RESOURCES_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Media CACHE PATH "Base Directory of resources" FORCE)

# Set the source path for the project
set (PROJ_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE PATH "Project SRC" FORCE)

# Add everything in the src folder
file(GLOB_RECURSE PROJ_SRC CONFIGURE_DEPENDS "${PROJ_SRC_PATH}/*.[ch]pp")

FetchContent_Declare(SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 2.6.x
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable (SFML ${PROJ_SRC} "SFML-Base.cpp" "SFML-Base.h" "src/LoadAssetThread.h")

#version tester
#add_executable (SFML ${PROJ_SRC} "test.cpp" "SFML-Base.h" "src/LoadAssetThread.h")

#if (MSVC)
 #   target_compile_options(SFML PRIVATE /std:c++20 /Zc:__cplusplus)
#else()
 #   target_compile_options(SFML PRIVATE -std=c++20)
#endif()

set(CMAKE_CXX_STANDARD 20)               # Force C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)      # Don’t fall back silently
set(CMAKE_CXX_EXTENSIONS OFF)            # Disable compiler extensions


# Make MSVC strict and report proper __cplusplus (minimal, safe)
if(MSVC)
  add_compile_options(/permissive- /Zc:__cplusplus)
endif()

if(MSVC)
  set_property(TARGET SFML PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Change Target C++ compile to ver 20
target_compile_features(SFML PRIVATE cxx_std_20)
# Link the SFML graphics lib
target_link_libraries(SFML PRIVATE sfml-graphics)

add_custom_command(
  TARGET SFML POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Media # dump all needed files here
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Media
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SFML PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
