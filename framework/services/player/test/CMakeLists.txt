cmake_minimum_required(VERSION 3.14)

project(cvi_player_service_test)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install/)
set(TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../)

include(${TOP_DIR}/cmake/googletest.cmake)

set(PLAYER_SERVICE_DIR ${TOP_DIR}/framework/services/player)
set(PLAYER_SERVICE_INCLUDE_DIR ${PLAYER_SERVICE_DIR}/include)

set(PLAYER_DIR ${TOP_DIR}/framework/components/player)
set(PLAYER_INCLUDE_DIR ${PLAYER_DIR}/include)

set(DEMUXER_DIR ${TOP_DIR}/framework/components/demuxer)
set(DEMUXER_INCLUDE_DIR ${DEMUXER_DIR}/include)

set(FILE_RECOVER_DIR ${TOP_DIR}/framework/components/file_recover)
set(FILE_RECOVER_INCLUDE_DIR ${FILE_RECOVER_DIR}/include)

set(FFMPEG_DIR ${TOP_DIR}/cpsl/third_party/ffmpeg)
set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib)

set(CVI_LOG_DIR ${TOP_DIR}/cpsl/common/log)
set(CVI_LOG_INCLUDE_DIR ${CVI_LOG_DIR}/include)
set(CVI_LOG_LIB_DIR ${CVI_LOG_DIR}/lib)

set(MAPI_DIR ${TOP_DIR}/framework/mapi/)
set(MAPI_INCLUDE_DIR ${MAPI_DIR}/include)

set(MIDDLEWARE_DIR ${TOP_DIR}/cpsl/mmf)
set(MIDDLEWARE_INCLUDE_DIR ${MIDDLEWARE_DIR}/include)

set(SYSUTILS_DIR ${TOP_DIR}/cpsl/common/sysutils)

set(HAL_DIR ${TOP_DIR}/cpsl/hal)

set(3RD_PARTY_LIB_DIR ${MIDDLEWARE_LIB_DIR}/3rd)
set(3RD_PARTY_LIBS
    -lcvi_RES
    -ltinyalsa
)

set(OSAL_DIR ${TOP_DIR}/cpsl/osal)
set(OSAL_INCLUDE_DIR ${OSAL_DIR}/include)

set(MQ_DIR ${TOP_DIR}/cpsl/common/mq)

set(SIGNAL_SLOT_DIR ${TOP_DIR}/cpsl/common/signal_slot)
set(SIGNAL_SLOT_INCLUDE_DIR ${SIGNAL_SLOT_DIR}/include)

set(ESSENTIAL_LIBS
    -ldl
    -lpthread
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit_test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME}
PRIVATE
    ${PLAYER_SERVICE_INCLUDE_DIR}
    ${PLAYER_INCLUDE_DIR}
    ${DEMUXER_INCLUDE_DIR}
    ${FILE_RECOVER_INCLUDE_DIR}
    ${MIDDLEWARE_INCLUDE_DIR}
    ${MAPI_INCLUDE_DIR}
    ${OSAL_INCLUDE_DIR}
    ${SIGNAL_SLOT_INCLUDE_DIR}
    ${CVI_LOG_INCLUDE_DIR}
)
target_link_directories(${PROJECT_NAME}
PRIVATE
    ${PLAYER_SERVICE_LIB_DIR}
    ${PLAYER_LIB_DIR}
    ${DEMUXER_LIB_DIR}
    ${FILE_RECOVER_LIB_DIR}
    ${FFMPEG_LIB_DIR}
    ${SYSUTILS_LIB_DIR}
    ${MAPI_LIB_DIR}
    ${MIDDLEWARE_LIB_DIR}
    ${HAL_SCREEN_LIB_DIR}
    ${HAL_GPIO_LIB_DIR}
    ${HAL_PWM_LIB_DIR}
    ${3RD_PARTY_LIB_DIR}
    ${OSAL_LIB_DIR}
    ${MQ_LIB_DIR}
    ${SIGNAL_SLOT_LIB_DIR}
    ${CVI_LOG_LIB_DIR}
)
target_link_libraries(${PROJECT_NAME}
    ${PLAYER_SERVICE_LIBS}
    ${PLAYER_LIBS}
    ${FILE_RECOVER_LIBS}
    ${SYSUTILS_LIBS}
    ${MAPI_LIBS}
    ${MIDDLEWARE_LIBS}
    ${HAL_SCREEN_LIBS}
    ${HAL_GPIO_LIBS}
    ${HAL_PWM_LIBS}
    ${3RD_PARTY_LIBS}
    ${OSAL_LIBS}
    ${MQ_LIBS}
    ${SIGNAL_SLOT_LIBS}
    ${ESSENTIAL_LIBS}
    gtest
    gtest_main
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
