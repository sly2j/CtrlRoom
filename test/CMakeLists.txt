# require ROOT 
find_package(ROOT REQUIRED)
include_directories(${ROOT_INCLUDE_DIRS})

add_executable(caen_v1729_pulse caen_v1729/pulse.cpp)
target_link_libraries(caen_v1729_pulse 
                      ctrlroom
                      ${Boost_LIBRARIES}
                      ${CAENVME_LIBRARIES}
                      ${ROOT_LIBRARIES})
