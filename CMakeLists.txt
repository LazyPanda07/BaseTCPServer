cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)

if (UNIX)
	add_definitions(-D__LINUX__)
endif(UNIX)

project(BaseTCPServer VERSION 1.17.0)

add_library(
	${PROJECT_NAME} STATIC
	src/BaseTCPServer.cpp
	src/WebServerException.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	include
)

if (DEFINED ENV{MARCH} AND NOT "$ENV{MARCH}" STREQUAL "")
	target_compile_options(${PROJECT_NAME} PRIVATE -march=$ENV{MARCH})
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY include DESTINATION .)
