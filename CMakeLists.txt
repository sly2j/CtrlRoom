# CMake project file for CtrlRoom v0.1

cmake_minimum_required (VERSION 2.6)
project (ctrlroom)

## make sure that the default is RELEASE
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RELEASE CACHE STRING
      "Choose the type of build, options are: None Debug Release."
      FORCE)
endif (NOT CMAKE_BUILD_TYPE)

## extra cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
    "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# default installation
get_filename_component (default_prefix "/usr/local" ABSOLUTE)
set (CMAKE_INSTALL_PREFIX ${default_prefix} CACHE STRING
      "Choose the installation directory; by default it installs in the NORMA directory."
      FORCE)

# CXX FLAGS (currently only supports gcc (>=4.8) and clang)
# -O1 for debug to take care of a problem with constexpr in gcc
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -fPIC")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -std=c++11 -fPIC")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fPIC")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fPIC")

# require BOOST library
find_package(Boost COMPONENTS program_options REQUIRED)

## CAENVME libraries required, except  for local development on a macbook, 
## where the VME libraries aren't present
IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CAENVME_INCLUDE_DIRS tmpincludes)
    set(CAENVME_DEFINITIONS -DLINUX)
ELSE ()
    find_package(CAENVME REQUIRED)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# include directories
include_directories(${PROJECT_SOURCE_DIR}
                    ${Boost_INCLUDE_DIRS}
                    ${CAENVME_INCLUDE_DIRS})

# defines
add_definitions(${CAENVME_DEFINITIONS})

add_subdirectory(ctrlroom/util)
add_subdirectory(ctrlroom/vme)

#IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # do nothing
    #ELSE ()
    add_subdirectory(test)
    #ENDIF ()

add_library(ctrlroom SHARED ctrlroom/board.cpp)
target_link_libraries(ctrlroom util vme
                      ${Boost_LIBRARIES}
                      ${CAENVME_LIBRARIES})
