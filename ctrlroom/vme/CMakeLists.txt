file (GLOB VME_SOURCES *.c* */*.c*)
file (GLOB VME_HEADERS *.h* */*.h*)

add_library(vme ${VME_SOURCES})

install (FILES ${VME_HEADERS} DESTINATION include/ctrlroom/vme)
