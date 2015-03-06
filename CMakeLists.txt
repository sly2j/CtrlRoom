# CMake project file for CtrlRoom v0.1

################################################################################
# PROJECT: CtrlRoom
################################################################################
cmake_minimum_required (VERSION 2.6)
project (CtrlRoom)
set (VERSION "1")
set (SOVERSION "1.0")

################################################################################
# Sources and headers
################################################################################
set (SOURCES "ctrlroom/vme/caen_v1729.cpp"
             "ctrlroom/vme/caen_bridge.cpp"
             "ctrlroom/vme/master.cpp"
             "ctrlroom/vme/caen_v1729/spec.cpp"
             "ctrlroom/util/io.cpp"
             "ctrlroom/util/logger.cpp"
             "ctrlroom/util/configuration.cpp"
             "ctrlroom/util/io/array.cpp"
             "ctrlroom/board.cpp")
set (HEADERS "ctrlroom/vme/master/block_transfer.hpp"
             "ctrlroom/vme/slave.hpp"
             "ctrlroom/vme/master.hpp"
             "ctrlroom/vme/caen_bridge.hpp"
             "ctrlroom/vme/caen_v1729.hpp"
             "ctrlroom/vme/caen_v1729/channel_index.hpp"
             "ctrlroom/vme/caen_v1729/spec.hpp"
             "ctrlroom/vme/vme64.hpp"
             "ctrlroom/util/root.hpp"
             "ctrlroom/util/stringify.hpp"
             "ctrlroom/util/assert.hpp"
             "ctrlroom/util/type_traits.hpp"
             "ctrlroom/util/io/array.hpp"
             "ctrlroom/util/mixin.hpp"
             "ctrlroom/util/configuration.hpp"
             "ctrlroom/util/exception.hpp"
             "ctrlroom/util/logger.hpp"
             "ctrlroom/util/io.hpp"
             "ctrlroom/board.hpp")

################################################################################
# CMAKE and Compiler Settings
################################################################################
## make sure that the default is RELEASE
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RELEASE CACHE STRING
      "Choose the type of build, options are: None Debug Release."
      FORCE)
endif (NOT CMAKE_BUILD_TYPE)
## extra cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
    "${CMAKE_SOURCE_DIR}/cmake/Modules/")
# CXX FLAGS (currently only supports gcc (>=4.8) and clang)
# -O1 for debug to take care of a problem with constexpr in gcc
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -fPIC")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -std=c++11 -fPIC")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fPIC")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fPIC")
# Project source dir is in the include path
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

################################################################################
# External Libraries
################################################################################
# require BOOST program_options library
find_package(Boost COMPONENTS program_options filesystem REQUIRED)
include_directories(AFTER ${Boost_INCLUDE_DIRS})

## CAENVME libraries required, except  for local development on a macbook, 
## where the VME libraries aren't present
IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CAENVME_INCLUDE_DIRS tmpincludes)
    set(CAENVME_DEFINITIONS -DLINUX)
ELSE ()
    find_package(CAENVME REQUIRED)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
include_directories(AFTER ${CAENVME_INCLUDE_DIRS})
add_definitions(${CAENVME_DEFINITIONS})

################################################################################
# Compile and Link
################################################################################
add_library(ctrlroom SHARED ${SOURCES} ${HEADERS})
target_link_libraries(ctrlroom 
                      ${Boost_LIBRARIES}
                      ${CAENVME_LIBRARIES})
set_target_properties(ctrlroom PROPERTIES VERSION ${VERSION} SOVERSION ${SOVERSION})

################################################################################
# Install
################################################################################
# 1. install all headers
FOREACH (header ${HEADERS})
  GET_FILENAME_COMPONENT(header_path ${header} PATH)
  install (FILES ${header} DESTINATION include/${header_path})
ENDFOREACH()
# 2. install the shared library
install (TARGETS ctrlroom DESTINATION lib)
