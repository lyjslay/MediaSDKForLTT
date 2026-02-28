cmake_minimum_required(VERSION 3.13)

project(
    cvi_demuxer
    VERSION 1.0.4
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
set(FFMPEG_LIBS
    -lavformat
    -lavcodec
)

set(CVI_LOG_DIR ${TOP_DIR}/cpsl/common/log)
set(CVI_LOG_INCLUDE_DIR ${CVI_LOG_DIR}/include)

set(ESSENTIAL_LIBS
    -lstdc++fs
)

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)
file(GLOB_RECURSE PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

# build shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
PRIVATE
    ${FFMPEG_INCLUDE_DIR}
    ${CVI_LOG_INCLUDE_DIR}
)
target_link_directories(${PROJECT_NAME}
PUBLIC
    ${FFMPEG_LIB_DIR}
    ${CVI_LOG_LIB_DIR}
)
target_link_libraries(${PROJECT_NAME}
PRIVATE
    ${FFMPEG_LIBS}
    ${CVI_LOG_LIBS}
    ${ESSENTIAL_LIBS}
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
        ${CVI_LOG_INCLUDE_DIR}
    )
    target_link_directories(${PROJECT_NAME}_static
    PUBLIC
        ${FFMPEG_LIB_DIR}
        ${CVI_LOG_LIB_DIR}
    )
    target_link_libraries(${PROJECT_NAME}_static
    PRIVATE
        ${FFMPEG_LIBS}
        ${CVI_LOG_LIBS}
        ${ESSENTIAL_LIBS}
    )
    set_target_properties(${PROJECT_NAME}_static
        PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
        PUBLIC_HEADER "${PUBLIC_HEADERS}"
    )
    install(TARGETS ${PROJECT_NAME}_static
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
    )
endif()

# build test
if (${PROJECT_NAME}_BUILD_TEST)
    add_subdirectory(test)
endif()
