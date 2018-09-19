# Only tested in 3.1, may work in older versions.
cmake_minimum_required(VERSION 3.1)

enable_language(CXX)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive -pthread " )
SET( CMAKE_EXE_LINKER_FLAGS "-pthread" )

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

project(PENDULUM C CXX)
set( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")

INCLUDE_DIRECTORIES( ${CTRLSANDBOX_SOURCE_DIR}/lib/include )

find_package(SFML 2 COMPONENTS system window graphics REQUIRED)

include_directories( ${SFML_INCLUDE_DIR} )
add_executable( pendulum main.cc InvPendulumEngine.cc )

target_link_libraries( pendulum ${SFML_LIBRARIES} )
