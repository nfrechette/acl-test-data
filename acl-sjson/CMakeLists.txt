cmake_minimum_required (VERSION 3.2)
project(acl-sjson CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/../acl-sjson-core/includes")
include_directories("${PROJECT_SOURCE_DIR}/../acl-v2.0-shim/includes")
include_directories("${PROJECT_SOURCE_DIR}/../acl-v2.1-shim/includes")

# Grab all of our source files
file(GLOB_RECURSE ALL_MAIN_SOURCE_FILES LIST_DIRECTORIES false
	${PROJECT_SOURCE_DIR}/sources/*.cpp
	${PROJECT_SOURCE_DIR}/sources/*.h
	${PROJECT_SOURCE_DIR}/*.md
	${PROJECT_SOURCE_DIR}/docs/*.md
	${PROJECT_SOURCE_DIR}/*.py)

create_source_groups("${ALL_MAIN_SOURCE_FILES}" ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${ALL_TEST_SOURCE_FILES} ${ALL_MAIN_SOURCE_FILES})

setup_default_compiler_flags(${PROJECT_NAME})

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE acl-sjson-core)
target_link_libraries(${PROJECT_NAME} PRIVATE acl-v20-shim)
target_link_libraries(${PROJECT_NAME} PRIVATE acl-v21-shim)

# Abort on failure, easier to debug issues this way
add_definitions(-DACL_ON_ASSERT_ABORT)
add_definitions(-DRTM_ON_ASSERT_ABORT)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
