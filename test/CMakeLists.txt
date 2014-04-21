# require BOOST library
find_package(Boost COMPONENTS program_options REQUIRED)

# get ROOT libraries and include directory
execute_process(COMMAND root-config --incdir
                OUTPUT_VARIABLE ROOT_INCLUDE_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND root-config --libs
                OUTPUT_VARIABLE ROOT_LIBRARIES 
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# include directories
include_directories(${ROOT_INCLUDE_DIR})

add_executable(caen_v1729_pulse caen_v1729/pulse.cpp)
