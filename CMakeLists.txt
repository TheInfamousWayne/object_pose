cmake_minimum_required(VERSION 3.5.1)
project (object_pose)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread" )
#link_libraries(stdc++fs)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(PythonLibs 2.7)
find_package(Armadillo REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})

file(GLOB sources ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${CMAKE_SOURCE_DIR}/include/*.hpp)

include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )

link_directories( ${CMAKE_BINARY_DIR}/bin)
link_directories( ${Boost_LIBRARY_DIRS} )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable( object_pose ${sources} ${headers} )

target_link_libraries( object_pose ${OpenCV_LIBS} ${Boost_LIBRARIES} ${ARMADILLO_LIBRARIES})
target_include_directories(object_pose PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(object_pose ${PYTHON_LIBRARIES})
