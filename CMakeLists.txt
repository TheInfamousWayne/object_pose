cmake_minimum_required(VERSION 3.16.4) 
project (object_pose)
find_package(OpenCV REQUIRED )

file(GLOB sources ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${CMAKE_SOURCE_DIR}/header/*.hpp)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )
link_directories( ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable( object_pose ${sources} ${headers} )

target_link_libraries( object_pose ${OpenCV_LIBS})
