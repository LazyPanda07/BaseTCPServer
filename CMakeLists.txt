cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "")

if (UNIX)
	add_definitions(-D__LINUX__)
endif (UNIX)

project(BaseTCPServer VERSION 1.14.0)

add_library(
	${PROJECT_NAME} STATIC
	src/BaseTCPServer.cpp
	src/WebServerException.cpp
)

if (DEFINED ENV{MARCH} AND NOT "$ENV{MARCH}" STREQUAL "")
	target_compile_options(${PROJECT_NAME} PRIVATE -march=$ENV{MARCH})
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include FILES_MATCHING PATTERN "*.h")
