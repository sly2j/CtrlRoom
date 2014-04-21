# try to find the ROOT  libraries and headers
# Once done, this will define:
#   * ROOT_FOUND - System has ROOT
#   * ROOT_INCLUDE_DIRS - The ROOT include directories
#   * ROOT_LIBRARIES - The ROOT libraries
#   * ROOT_DEFINITIONS - the ROOT definitions

# just use root-config
execute_process(COMMAND root-config --incdir
                OUTPUT_VARIABLE ROOT_INCLUDE_DIRS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs
                OUTPUT_VARIABLE ROOT_LIBRARIES 
                OUTPUT_STRIP_TRAILING_WHITESPACE)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ROOT_FOUND to TRUE
find_package_handle_standard_args(ROOT DEFAULT_MSG
                                  ROOT_LIBRARIES ROOT_INCLUDE_DIRS)

mark_as_advanced(ROOT_INCLUDE_DIR ROOT_LIBRARY)
