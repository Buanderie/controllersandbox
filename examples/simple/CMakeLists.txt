cmake_minimum_required(VERSION 2.8)
project( Test )

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive -pthread " )
SET( CMAKE_EXE_LINKER_FLAGS "-pthread" )

#
#
## CHECK FOR COMPILER CAPABILITIES
#
#
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
##

find_package( OpenCV REQUIRED )

include_directories( ${CTRLSANDBOX_SOURCE_DIR}/lib/include )
# find_package( OpenCV REQUIRED )
add_executable( ctrlsb main.cc )
target_link_libraries( ctrlsb ${OpenCV_LIBS} )

