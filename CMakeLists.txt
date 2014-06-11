# CMake project file for CtrlRoom v0.1

cmake_minimum_required (VERSION 2.6)
project (ctrlroom)
set (VERSION "1")
set (SOVERSION "1.0")

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

# require BOOST library
find_package(Boost COMPONENTS program_options filesystem REQUIRED)

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

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # do nothing
ELSE ()
    add_subdirectory(test)
ENDIF ()

file (GLOB CTRLROOM_SOURCES ctrlroom/*.c*)
file (GLOB CTRLROOM_HEADERS ctrlroom/*.h*)

add_library(ctrlroom SHARED ${CTRLROOM_SOURCES})
target_link_libraries(ctrlroom util vme
                      ${Boost_LIBRARIES}
                      ${CAENVME_LIBRARIES})

set_target_properties(ctrlroom PROPERTIES VERSION ${VERSION} SOVERSION ${SOVERSION})

install (FILES ${CTRLROOM_HEADERS} DESTINATION include/ctrlroom)
install (TARGETS ctrlroom DESTINATION lib)
