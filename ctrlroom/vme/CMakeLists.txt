file (GLOB VME_MAIN_HEADERS *.hpp)
file (GLOB VME_SUBDIR_CAEN_V1729_HEADERS caen_v1729/*.hpp)
file (GLOB VME_SUBDIR_MASTER_HEADERS master/*.hpp)

install (FILES ${VME_MAIN_HEADERS} DESTINATION include/ctrlroom/vme)
install (FILES ${VME_SUBDIR_MASTER_HEADERS} DESTINATION include/ctrlroom/vme/master)
install (FILES ${VME_SUBDIR_CAEN_V1729_HEADERS} DESTINATION include/ctrlroom/vme/caen_v1729)
