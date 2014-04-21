# try to find the caen VME libraries and headers
# Once done, this will define:
#   * CAENVME_FOUND - System has CAENVME
#   * CAENVME_INCLUDE_DIRS - The CAENVME include directories
#   * CAENVME_LIBRARIES - The CAENVME libraries
#   * CAENVME_DEFINITIONS - the CAENVME definitions

find_path(CAENVME_INCLUDE_DIR CAENVMElib.h
          HINTS ${PROJECT_SOURCE_DIR}/CAEN ${PROJECT_SOURCE_DIR}/CAENVME 
          /usr/include/CAEN /usr/local/include/CAEN
          /usr/include/CAENVME /usr/local/include/CAENVME
          /usr/include /usr/local/include)

find_library(CAENVME_LIBRARY CAENVME
             HINTS ${PROJECT_SOURCE_DIR}/lib
             /usr/local/lib/CAENVME /usr/local/lib/CAEN /usr/local/lib
             /usr/lib/CAENVME /usr/lib/CAEN /usr/lib)

if (UNIX)
    set(CAENVME_DEFINITIONS -DLINUX)
else ()
    set(CAENVME_DEFINITIONS -DWIN32)
endif (UNIX)

set(CAENVME_INCLUDE_DIRS ${CAENVME_INCLUDE_DIR})
set(CAENVME_LIBRARIES ${CAENVME_LIBRARY})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set CAENVME_FOUND to TRUE
find_package_handle_standard_args(CAENVME DEFAULT_MSG
                                  CAENVME_LIBRARY CAENVME_INCLUDE_DIR)

mark_as_advanced(CAENVME_INCLUDE_DIR CAENVME_LIBRARY)
