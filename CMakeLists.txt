# CMake project file for CtrlRoom v0.1

cmake_minimum_required (VERSION 2.6)
project (ctrlroom)

## make sure that the default is RELEASE
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RELEASE CACHE STRING
      "Choose the type of build, options are: None Debug Release."
      FORCE)
endif (NOT CMAKE_BUILD_TYPE)

# default installation
get_filename_component (default_prefix "/usr/local" ABSOLUTE)
set (CMAKE_INSTALL_PREFIX ${default_prefix} CACHE STRING
      "Choose the installation directory; by default it installs in the NORMA directory."
      FORCE)

## set OS dependend compiler flags
if (UNIX)
    add_definitions(-DLINUX)
else ()
    add_definitions(-DWIN32)
endif (UNIX)

# CXX FLAGS (currently only supports g++ and clang++)
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")

# require BOOST library
find_package(Boost COMPONENTS program_options REQUIRED)

# include directories
include_directories(${PROJECT_SOURCE_DIR}
                    ${Boost_INCLUDE_DIRS}
                    tmpincludes) ## <== only for debug builds

## for local development on a macbook, where the VME libraries
## aren't present
IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    include_directories(tmpincludes)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


add_subdirectory(ctrlroom/util)
add_subdirectory(ctrlroom/vme)

add_library(ctrlroom SHARED ctrlroom/board.cpp)
target_link_libraries(ctrlroom util vme
                      ${Boost_LIBRARIES})
