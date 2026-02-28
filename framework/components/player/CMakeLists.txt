cmake_minimum_required(VERSION 3.13)

project(
    cvi_player
    VERSION 1.0.7
    LANGUAGES C CXX
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install/)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

include(${TOP_DIR}/cmake/compile_options.cmake)
include(${TOP_DIR}/cmake/summary.cmake)

set(FFMPEG_DIR ${TOP_DIR}/cpsl/third_party/ffmpeg)
set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)
set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib)
set(FFMPEG_LIBS
    -lavformat
    -lavcodec
    -lavutil
    -lswresample
)

set(CVI_DEMUXER_DIR ${TOP_DIR}/framework/components/demuxer)
set(CVI_DEMUXER_INCLUDE_DIR ${CVI_DEMUXER_DIR}/include)

set(CVI_LOG_DIR ${TOP_DIR}/cpsl/common/log)
set(CVI_LOG_INCLUDE_DIR ${CVI_LOG_DIR}/include)

set(CVI_MEMORY_ALLOCATOR_DIR ${TOP_DIR}/framework/components/mem_alloc)
set(CVI_MEMORY_ALLOCATOR_INCLUDE_DIR ${CVI_MEMORY_ALLOCATOR_DIR}/include)

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

# build shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CVI_DEMUXER_INCLUDE_DIR}
PRIVATE
    ${FFMPEG_INCLUDE_DIR}
    ${CVI_LOG_INCLUDE_DIR}
    ${CVI_MEMORY_ALLOCATOR_INCLUDE_DIR}
)
target_link_directories(${PROJECT_NAME}
PUBLIC
    ${FFMPEG_LIB_DIR}
    ${CVI_DEMUXER_LIB_DIR}
    ${CVI_LOG_LIB_DIR}
    ${CVI_MEMORY_ALLOCATOR_LIB_DIR}
)
target_link_libraries(${PROJECT_NAME}
PUBLIC
    ${CVI_DEMUXER_LIBS}
PRIVATE
    ${FFMPEG_LIBS}
    ${CVI_LOG_LIBS}
    ${CVI_MEMORY_ALLOCATOR_LIBS}
)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

# build static library if need
if (${PROJECT_NAME}_BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME}_static STATIC ${SOURCES})
    target_include_directories(${PROJECT_NAME}_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CVI_DEMUXER_INCLUDE_DIR}
    PRIVATE
        ${FFMPEG_INCLUDE_DIR}
        ${CVI_LOG_INCLUDE_DIR}
        ${CVI_MEMORY_ALLOCATOR_INCLUDE_DIR}
    )
    target_link_directories(${PROJECT_NAME}_static
    PUBLIC
        ${FFMPEG_LIB_DIR}
        ${CVI_DEMUXER_LIB_DIR}
        ${CVI_LOG_LIB_DIR}
        ${CVI_MEMORY_ALLOCATOR_LIB_DIR}
    )
    target_link_libraries(${PROJECT_NAME}_static
    PUBLIC
        ${CVI_DEMUXER_LIBS}
    PRIVATE
        ${FFMPEG_LIBS}
        ${CVI_LOG_LIBS}
        ${CVI_MEMORY_ALLOCATOR_LIBS}
    )
    set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    install(TARGETS ${PROJECT_NAME}_static ARCHIVE DESTINATION lib)
endif()

# build test
if (${PROJECT_NAME}_BUILD_TEST)
    add_subdirectory(test)
endif()

# install export headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING
        PATTERN "*.h"
)
