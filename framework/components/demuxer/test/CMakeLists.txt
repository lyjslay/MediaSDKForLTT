cmake_minimum_required(VERSION 3.11.4)

project(${CMAKE_PROJECT_NAME}_test)

include(${TOP_DIR}/cmake/googletest.cmake)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit_test.cpp
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
