file (GLOB UTIL_SOURCES *.cpp */*.cpp PARENT_SCOPE)
file (GLOB UTIL_MAIN_HEADERS *.hpp)
file (GLOB UTIL_SUBDIR_IO_HEADERS io/*.hpp)

install(FILES ${UTIL_MAIN_HEADERS} DESTINATION include/ctrlroom/util)
install(FILES ${UTIL_SUBDIR_IO_HEADERS} DESTINATION include/ctrlroom/util/io)
