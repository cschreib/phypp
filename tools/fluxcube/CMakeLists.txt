cmake_minimum_required(VERSION 2.6)
project(fluxcube)

include_directories(${VIF_INCLUDE_DIRS})
add_executable(fluxcube fluxcube.cpp)
target_link_libraries(fluxcube ${VIF_LIBRARIES})
install(TARGETS fluxcube DESTINATION bin)
