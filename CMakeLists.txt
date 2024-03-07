cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)

if (UNIX)
	add_definitions(-D__LINUX__)
endif (UNIX)

project(BaseTCPServer)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install-dir CACHE STRING "")

add_library(
	${PROJECT_NAME} STATIC
	src/BaseTCPServer.cpp
	src/ClientData.cpp
	src/WebException.cpp
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_DIR}/lib)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_DIR}/include FILES_MATCHING PATTERN "*.h")
endif ()
