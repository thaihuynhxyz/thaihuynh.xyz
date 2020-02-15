cmake_minimum_required(VERSION 2.8)
project(thaihuynh_xyz)

set(CMAKE_C_STANDARD 99)

# link library
link_directories(/usr/lib/x86_64-linux-gnu/)

# link header
include_directories(/usr/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

# def source
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/ MAIN_SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/util/ UTIL_SOURCE_FILES)
aux_source_directory(src/service/ SERVICE_SOURCE_FILES)

# link resource
add_custom_target(
        ${PROJECT_NAME}_res ALL
        ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/blog ${CMAKE_BINARY_DIR}/blog
        SOURCES ${${PROJECT_NAME}_res}
)

add_executable(${PROJECT_NAME} ${MAIN_SOURCE_FILES}
        ${UTIL_SOURCE_FILES}
        ${SERVICE_SOURCE_FILES})

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_res)

target_link_libraries(${PROJECT_NAME} microhttpd)