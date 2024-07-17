cmake_minimum_required(VERSION 3.21)
project(SteelMillSlab)

set(CMAKE_CXX_STANDARD 11)
include_directories("/opt/homebrew/Cellar/gecode/6.2.0_1/include")
link_directories("/opt/homebrew/Cellar/gecode/6.2.0_1/lib")

add_executable(SteelMillSlab mainSteelMillSlab.cpp)
target_link_libraries(SteelMillSlab gecodedriver)
target_link_libraries(SteelMillSlab gecodegist)
target_link_libraries(SteelMillSlab gecodesearch)
target_link_libraries(SteelMillSlab gecodeminimodel)
target_link_libraries(SteelMillSlab gecodeint)
target_link_libraries(SteelMillSlab gecodekernel)
target_link_libraries(SteelMillSlab gecodesupport)




