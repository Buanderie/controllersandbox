# Only tested in 3.1, may work in older versions.
cmake_minimum_required(VERSION 3.1)

project(GNGUI)
set( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")

find_package(OpenGL REQUIRED)
find_package(SFML 2 COMPONENTS system window graphics REQUIRED)

include_directories( ${SFML_INCLUDE_DIR} )
add_executable(gng_ui main.cpp )

target_link_libraries( gng_ui ${OPENGL_LIBRARIES} ${SFML_LIBRARIES} )
