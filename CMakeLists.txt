cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE STRING "")

if (UNIX)
	add_definitions(-D__LINUX__)
endif (UNIX)

project(BaseTCPServer)

add_library(
	${PROJECT_NAME} STATIC
	src/BaseTCPServer.cpp
	src/WebException.cpp
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include FILES_MATCHING PATTERN "*.h")
