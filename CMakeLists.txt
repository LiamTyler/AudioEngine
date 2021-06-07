cmake_minimum_required(VERSION 3.13) #target_link_directories

Project(AudioEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LINUX_PROGRAM   "NOT_IN_USE")
set(WINDOWS_PROGRAM "NOT_IN_USE")

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE )
endif()

if(UNIX AND NOT APPLE)
    set(LINUX_PROGRAM   "IN_USE")
elseif(WIN32)
    set(WINDOWS_PROGRAM "IN_USE")
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set_property(
    DIRECTORY
    PROPERTY
    COMPILE_DEFINITIONS
    $<$<CONFIG:Debug>:CMAKE_DEFINE_DEBUG_BUILD>
    $<$<CONFIG:Release>:CMAKE_DEFINE_RELEASE_BUILD>
)

configure_file(cmake/platform_defines.hpp.in src/platform_defines.hpp)

set(SRC
    src/core_defines.hpp
    src/platform_defines.hpp
    src/utils/logger.cpp
    src/utils/logger.hpp
    src/main.cpp
)

# configure and build portaudio into the build/ext/portaudio directory
# This happens now, at configuration time of this top level cmake file
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ext/portaudio)
execute_process(
    COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR}/ext/portaudio
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ext/portaudio
)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/ext/portaudio
)

add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/ext/portaudio)
target_link_libraries(${PROJECT_NAME} PRIVATE portaudio)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ext/portaudio/include
    src/
)
