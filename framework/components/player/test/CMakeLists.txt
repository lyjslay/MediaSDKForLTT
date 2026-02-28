cmake_minimum_required(VERSION 3.14)

project(${CMAKE_PROJECT_NAME}_test)

include(${TOP_DIR}/cmake/googletest.cmake)

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    ${CMAKE_PROJECT_NAME}
    -lpthread
    gtest
    gtest_main
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
