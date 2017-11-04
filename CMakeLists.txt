cmake_minimum_required(VERSION 2.8)
project(thaihuynh_xyz)

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
#add_custom_target(
#        ${PROJECT_NAME}_res ALL
#        ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
#        SOURCES ${${PROJECT_NAME}_res}
#)

add_executable(${PROJECT_NAME} ${MAIN_SOURCE_FILES}
        ${UTIL_SOURCE_FILES}
        ${SERVICE_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} microhttpd)