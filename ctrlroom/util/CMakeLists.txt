file (GLOB UTIL_SOURCES *.c* */*.c*)
file (GLOB UTIL_HEADERS *.h* */*.h*)

add_library(util ${UTIL_SOURCES})

install(FILES ${UTIL_HEADERS} DESTINATION include/ctrlroom/util)
