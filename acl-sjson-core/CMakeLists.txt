cmake_minimum_required (VERSION 3.2)
project(acl-sjson-core CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/includes")

# Grab all of our source files
file(GLOB_RECURSE ALL_MAIN_SOURCE_FILES LIST_DIRECTORIES false
	${PROJECT_SOURCE_DIR}/sources/*.cpp
	${PROJECT_SOURCE_DIR}/sources/*.h
	${PROJECT_SOURCE_DIR}/includes/*.h)

create_source_groups("${ALL_MAIN_SOURCE_FILES}" ${PROJECT_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${ALL_MAIN_SOURCE_FILES})

setup_default_compiler_flags(${PROJECT_NAME})
