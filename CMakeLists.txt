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
include(FetchContent)

# Add debug option
option(BUILD_DEBUG "Enable debuging" OFF)
if(BUILD_DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Set the default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Declare GoogleTest fetch content (only when tests are enabled)
if(BUILD_TESTS)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
  )
  FetchContent_MakeAvailable(googletest)
endif()

add_definitions(-D__DATA_DIRECTORY__="root.tessergram.data/")

# Verbose compile option
option(VERBOSE "Enable verbose compile" OFF)
if(VERBOSE)
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

# Specify the source files
set(SOURCE_FILES
  src/debug.cpp
  src/fetch-api.cpp
  src/json-parser.cpp
  src/request.cpp
  src/telegram/common.cpp
  src/telegram/basic.cpp
  src/telegram/media.cpp
  src/telegram/webhook.cpp
  src/telegram/keyboard.cpp
  external/mongoose/src/mongoose.c
)

# Create static library
add_library(${PROJECT_NAME}-ar STATIC ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME}-ar PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
if(BUILD_DEBUG)
  set_target_properties(${PROJECT_NAME}-ar PROPERTIES COMPILE_OPTIONS "-g" LINK_OPTIONS "-g")
endif()

# Create shared library from static library
add_library(${PROJECT_NAME}-lib SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-ar>)
set_target_properties(${PROJECT_NAME}-lib PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
if(BUILD_DEBUG)
  set_target_properties(${PROJECT_NAME}-lib PROPERTIES COMPILE_OPTIONS "-g" LINK_OPTIONS "-g")
endif()

# Link library
target_link_libraries(${PROJECT_NAME}-lib ${CURL_LIBRARIES})

# Include directories for the project
set(INCLUDE_DIRS
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/mongoose/include>
  $<INSTALL_INTERFACE:include>
  ${CURL_INCLUDE_DIR}
)
target_include_directories(${PROJECT_NAME}-ar PUBLIC ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-lib PUBLIC ${INCLUDE_DIRS})

# Create example executable

## Main Example
add_executable(${PROJECT_NAME} examples/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-lib)
target_link_libraries(${PROJECT_NAME} PUBLIC -lpthread)

## Keyboard Example
add_executable(${PROJECT_NAME}-keyboard examples/keyboard.cpp)
target_link_libraries(${PROJECT_NAME}-keyboard PRIVATE ${PROJECT_NAME}-lib)
target_link_libraries(${PROJECT_NAME}-keyboard PUBLIC -lpthread)

## Inline Keyboard Example
add_executable(${PROJECT_NAME}-inline-keyboard examples/inline-keyboard.cpp)
target_link_libraries(${PROJECT_NAME}-inline-keyboard PRIVATE ${PROJECT_NAME}-lib)
target_link_libraries(${PROJECT_NAME}-inline-keyboard PUBLIC -lpthread)

# Add test configuration (only when tests are enabled)
if(BUILD_TESTS)
  add_executable(${PROJECT_NAME}-test test/test-sam-access.cpp)
  target_include_directories(${PROJECT_NAME}-test PUBLIC ${INCLUDE_DIRS} ${GTest_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME}-lib gtest gtest_main -lpthread)
  enable_testing()
  add_test(NAME example_test COMMAND ${PROJECT_NAME}-test)
endif()

# Compiler and linker flags
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fPIC")
set(CMAKE_C_FLAGS_RELEASE "-O3 -fPIC")