cmake_minimum_required(VERSION 3.13)

project(
    cvi_thumbnail_extractor
    VERSION 1.0.2
    LANGUAGES C CXX
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install/)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

set(TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

include(${TOP_DIR}/cmake/compile_options.cmake)
include(${TOP_DIR}/cmake/summary.cmake)

set(FFMPEG_DIR ${TOP_DIR}/cpsl/third_party/ffmpeg)
set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)
set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib)

set(DEMUXER_DIR ${TOP_DIR}/framework/components/demuxer)
set(DEMUXER_INCLUDE_DIR ${DEMUXER_DIR}/include)

set(RECOVER_DIR ${TOP_DIR}/framework/components/file_recover)
set(RECOVER_INCLUDE_DIR ${RECOVER_DIR}/include)

set(CVI_LOG_DIR ${TOP_DIR}/cpsl/common/log)
set(CVI_LOG_INCLUDE_DIR ${CVI_LOG_DIR}/include)

set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SOURCES
    ${SOURCES_DIR}/cvi_thumbnail_extractor.cpp
    ${SOURCES_DIR}/thumbnail_extractor.cpp
)

set(PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/cvi_thumbnail_extractor.h
)

# build shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${DEMUXER_INCLUDE_DIR}
    ${RECOVER_INCLUDE_DIR}
PRIVATE
    ${FFMPEG_INCLUDE_DIR}
    ${CVI_LOG_INCLUDE_DIR}
)
target_link_directories(${PROJECT_NAME}
PUBLIC
    ${FFMPEG_LIB_DIR}
    ${DEMUXER_LIB_DIR}
    ${CVI_LOG_LIB_DIR}
)
target_link_libraries(${PROJECT_NAME}
PRIVATE
    ${DEMUXER_LIBS}
    ${CVI_LOG_LIBS}
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
)

# build static library if need
if (${PROJECT_NAME}_BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME}_static STATIC ${SOURCES})
    target_include_directories(${PROJECT_NAME}_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${FFMPEG_INCLUDE_DIR}
        ${DEMUXER_INCLUDE_DIR}
        ${CVI_LOG_INCLUDE_DIR}
        ${RECOVER_INCLUDE_DIR}
    )
    target_link_directories(${PROJECT_NAME}_static
    PUBLIC
        ${FFMPEG_LIB_DIR}
        ${DEMUXER_LIB_DIR}
        ${CVI_LOG_LIB_DIR}
    )
    target_link_libraries(${PROJECT_NAME}_static
    PRIVATE
        ${DEMUXER_LIBS}
        ${CVI_LOG_LIBS}
    )
    set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    install(TARGETS ${PROJECT_NAME}_static
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
    )
endif()

# build test
if (${PROJECT_NAME}_BUILD_TEST)
    add_subdirectory(test)
endif()
