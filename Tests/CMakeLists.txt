cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)

if (UNIX)
	add_definitions(-D__LINUX__)
endif (UNIX)

project(Tests)

add_executable(
	${PROJECT_NAME}
	main.cpp
)

target_include_directories(
	${PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/../src/
)

target_link_directories(
	${PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/../BaseTCPServer/lib/
)

target_link_libraries(
	${PROJECT_NAME}
	BaseTCPServer
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/)
