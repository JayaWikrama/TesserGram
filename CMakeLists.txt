cmake_minimum_required(VERSION 3.0.0)

# Project configuration
project(tessergram LANGUAGES C CXX)

# Define versioning variables
set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# Definition for version
string(TIMESTAMP BUILD_YEAR "%Y")
string(TIMESTAMP BUILD_MONTH "%m")
string(TIMESTAMP BUILD_DAY "%d")
set(TESSER_GRAM_MODULE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${BUILD_YEAR}${BUILD_MONTH}${BUILD_DAY}")
add_definitions(-DTESSER_GRAM_MODULE_VERSION="${TESSER_GRAM_MODULE_VERSION}")

# Find and Check Library
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)

# Include necessary modules
include(FindPkgConfig)

# Add debug option
option(BUILD_DEBUG "Enable debuging" OFF)
if(BUILD_DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Set the default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Verbose compile option
option(VERBOSE "Enable verbose compile" OFF)
if(VERBOSE)
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

# Specify the source files
set(SOURCE_FILES
  src/json-validator.cpp
  src/fetch-api.cpp
  src/request.cpp
  src/polling-controller.cpp
  src/type/user.cpp
  src/type/chat.cpp
  src/type/media.cpp
  src/type/message.cpp
  src/type/callback-query.cpp
  src/telegram/node-message.cpp
  src/telegram/common.cpp
  src/telegram/basic.cpp
  src/telegram/media.cpp
  src/telegram/webhook.cpp
  src/telegram/keyboard.cpp
  external/mongoose/src/mongoose.c
)

# Create object
add_library(${PROJECT_NAME}-obj OBJECT ${SOURCE_FILES})

# Create static library
add_library(${PROJECT_NAME}-ar STATIC $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
set_target_properties(${PROJECT_NAME}-ar PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
if(BUILD_DEBUG)
  set_target_properties(${PROJECT_NAME}-ar PROPERTIES COMPILE_OPTIONS "-g" LINK_OPTIONS "-g")
endif()

# Create shared library from static library
add_library(${PROJECT_NAME}-lib SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
set_target_properties(${PROJECT_NAME}-lib PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
if(BUILD_DEBUG)
  set_target_properties(${PROJECT_NAME}-lib PROPERTIES COMPILE_OPTIONS "-g" LINK_OPTIONS "-g")
endif()

# Link library
target_link_libraries(${PROJECT_NAME}-lib)

# Include directories for the project
set(INCLUDE_DIRS
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/mongoose/include>
  $<INSTALL_INTERFACE:include>
  ${CURL_INCLUDE_DIR}
)
target_include_directories(${PROJECT_NAME}-obj PUBLIC ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-ar PUBLIC ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-lib PUBLIC ${INCLUDE_DIRS})

# Add dependencies
add_dependencies(${PROJECT_NAME}-lib utils-ar)

# Create example executable

## Main Example
add_executable(${PROJECT_NAME} examples/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-lib utils-ar)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CURL_LIBRARIES} -lpthread)

## Send Message Example
add_executable(${PROJECT_NAME}-receive-message examples/receive-message.cpp)
target_link_libraries(${PROJECT_NAME}-receive-message PRIVATE ${PROJECT_NAME}-lib utils-ar)
target_link_libraries(${PROJECT_NAME}-receive-message PUBLIC ${CURL_LIBRARIES} -lpthread)

## Send Message Example
add_executable(${PROJECT_NAME}-send-message examples/send-message.cpp)
target_link_libraries(${PROJECT_NAME}-send-message PRIVATE ${PROJECT_NAME}-lib utils-ar)
target_link_libraries(${PROJECT_NAME}-send-message PUBLIC ${CURL_LIBRARIES} -lpthread)

## Keyboard Example
add_executable(${PROJECT_NAME}-keyboard examples/keyboard.cpp)
target_link_libraries(${PROJECT_NAME}-keyboard PRIVATE ${PROJECT_NAME}-lib utils-ar)
target_link_libraries(${PROJECT_NAME}-keyboard PUBLIC ${CURL_LIBRARIES} -lpthread)

## Inline Keyboard Example
add_executable(${PROJECT_NAME}-inline-keyboard examples/inline-keyboard.cpp)
target_link_libraries(${PROJECT_NAME}-inline-keyboard PRIVATE ${PROJECT_NAME}-lib utils-ar)
target_link_libraries(${PROJECT_NAME}-inline-keyboard PUBLIC ${CURL_LIBRARIES} -lpthread)

## Media Example
add_executable(${PROJECT_NAME}-media examples/media.cpp)
target_link_libraries(${PROJECT_NAME}-media PRIVATE ${PROJECT_NAME}-lib utils-ar)
target_link_libraries(${PROJECT_NAME}-media PUBLIC ${CURL_LIBRARIES} -lpthread)

# Compiler and linker flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fPIC")
set(CMAKE_C_FLAGS_RELEASE "-O3 -fPIC")
add_definitions(-DCXXSTD_11)
add_definitions(-D__TESSERGRAM_DATA_DIRECTORY__="root.tessergram.data/")

# Add subdirectory for DataFrame
add_subdirectory(external/utils EXCLUDE_FROM_ALL)