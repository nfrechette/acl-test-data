cmake_minimum_required (VERSION 3.2)
project(acl-v21-shim CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/../acl-sjson-core/includes")
include_directories("${PROJECT_SOURCE_DIR}/../acl-v2.1-shim/includes")
include_directories("${PROJECT_SOURCE_DIR}/../external/acl_v21/includes")
include_directories("${PROJECT_SOURCE_DIR}/../external/acl_v21/external/rtm/includes")
include_directories("${PROJECT_SOURCE_DIR}/../external/acl_v21/external/sjson-cpp/includes")

# Grab all of our source files
file(GLOB_RECURSE ALL_MAIN_SOURCE_FILES LIST_DIRECTORIES false
	${PROJECT_SOURCE_DIR}/sources/*.cpp
	${PROJECT_SOURCE_DIR}/sources/*.h
	${PROJECT_SOURCE_DIR}/includes/*.h)

create_source_groups("${ALL_MAIN_SOURCE_FILES}" ${PROJECT_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${ALL_MAIN_SOURCE_FILES})

setup_default_compiler_flags(${PROJECT_NAME})

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE acl-sjson-core)

# Abort on failure, easier to debug issues this way
add_definitions(-DACL_ON_ASSERT_ABORT)
add_definitions(-DRTM_ON_ASSERT_ABORT)

# Enable version namespaces
add_definitions(-DACL_ENABLE_VERSION_NAMESPACE)
add_definitions(-DRTM_ENABLE_VERSION_NAMESPACE)
add_definitions(-DSJSON_CPP_ENABLE_VERSION_NAMESPACE)
