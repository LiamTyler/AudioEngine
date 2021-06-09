cmake_minimum_required(VERSION 3.13) #target_link_directories

Project(AudioEngine)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(set_postfix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LINUX_PROGRAM   "NOT_IN_USE")
set(WINDOWS_PROGRAM "NOT_IN_USE")

if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
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


set_property(
    DIRECTORY
    PROPERTY
    COMPILE_DEFINITIONS
    $<$<CONFIG:Debug>:CMAKE_DEFINE_DEBUG_BUILD>
    $<$<CONFIG:Release>:CMAKE_DEFINE_RELEASE_BUILD>
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/platform_defines.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/platform_defines.hpp)

set(SRC
    src/core_defines.hpp
    src/audio_file.hpp
	src/audio_system.cpp
    src/audio_system.hpp
    src/platform_defines.hpp
    src/utils/logger.cpp
    src/utils/logger.hpp
    src/main.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

# configure and build portaudio into the build/ext/portaudio directory
# This happens now, at configuration time of this top level cmake file
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ext/portaudio)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}/ext/portaudio
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ext/portaudio
)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build . --config Release
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ext/portaudio
)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ext/libsndfile)
execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}/ext/libsndfile
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ext/libsndfile
)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build . --config Release
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/ext/libsndfile
)
if(WIN32)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/ext/portaudio/Release/portaudio_x64.dll ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/ext/libsndfile/Release/sndfile.dll ${CMAKE_BINARY_DIR}/bin
    )
endif()

add_executable(${PROJECT_NAME} ${SRC})
SET_TARGET_POSTFIX( ${PROJECT_NAME})
target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/ext/portaudio
    ${CMAKE_BINARY_DIR}/ext/portaudio/Release
    ${CMAKE_BINARY_DIR}/ext/libsndfile/Release
)
target_link_libraries(${PROJECT_NAME} PRIVATE portaudio_x64 sndfile)
target_include_directories(${PROJECT_NAME} PUBLIC
    ext/portaudio/include
    ${CMAKE_BINARY_DIR}/ext/libsndfile/include
    src/
)
