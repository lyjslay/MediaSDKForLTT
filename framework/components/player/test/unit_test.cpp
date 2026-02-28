#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "cvi_player/cvi_player.h"

namespace {

constexpr char data_source[] = "test.mp4";

TEST(UnitTest, SetDataSourceTest) {
    CVI_PLAYER_Init();
    char temp_data_source[64] = "";
    CVI_PLAYER_HANDLE_T player = NULL;
    CVI_PLAYER_Create(&player);
    CVI_PLAYER_SetDataSource(player, data_source);
    int32_t ret = CVI_PLAYER_GetDataSource(player, temp_data_source);
    ASSERT_EQ(ret, 0);
    ret = strncmp(data_source, temp_data_source, strlen(data_source));
    EXPECT_EQ(ret, 0);
    CVI_PLAYER_Destroy(&player);
    EXPECT_EQ(player, nullptr);
    CVI_PLAYER_Deinit();
}

TEST(UnitTest, DestroyTest) {
    CVI_PLAYER_Init();
    CVI_PLAYER_HANDLE_T player = NULL;
    int32_t ret = CVI_PLAYER_Create(&player);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_Destroy(&player);
    ASSERT_EQ(ret, 0);
    EXPECT_EQ(player, nullptr);
    CVI_PLAYER_Deinit();
}

TEST(UnitTest, PlayTest) {
    CVI_PLAYER_Init();
    CVI_PLAYER_HANDLE_T player = NULL;
    CVI_PLAYER_Create(&player);
    int32_t ret = CVI_PLAYER_SetDataSource(player, data_source);
    ASSERT_EQ(ret, 0);
    CVI_PLAYER_MEDIA_INFO_S info = {};
    ret = CVI_PLAYER_GetMediaInfo(player, &info);
    ASSERT_EQ(ret, 0);
    EXPECT_NE(info.width, 0);
    EXPECT_NE(info.height, 0);
    ret = CVI_PLAYER_Play(player);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ret = CVI_PLAYER_Stop(player);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_Destroy(&player);
    ASSERT_EQ(ret, 0);
    EXPECT_EQ(player, nullptr);
    ret = CVI_PLAYER_Play(player);
    EXPECT_TRUE(ret != 0);
    CVI_PLAYER_Deinit();
}

TEST(UnitTest, CustomArgEventTest) {
    CVI_PLAYER_Init();
    CVI_PLAYER_HANDLE_T player = NULL;
    CVI_PLAYER_Create(&player);
    CVI_PLAYER_SetDataSource(player, data_source);
    struct CustomArg {
        int32_t index;
        bool pause_check;
        bool resume_check;
    };
    auto eventHandler = [] (void *arg, CVI_PLAYER_HANDLE_T player, CVI_PLAYER_EVENT_S *event) {
        CustomArg *custom_arg = static_cast<CustomArg *>(arg);
        switch(event->type) {
        case CVI_PLAYER_EVENT_PAUSE:
            EXPECT_EQ(custom_arg->index, 1);
            custom_arg->pause_check = true;
            break;
        case CVI_PLAYER_EVENT_RESUME:
            EXPECT_EQ(custom_arg->index, 1);
            custom_arg->resume_check = true;
            break;
        default:
            break;
        }
    };
    CustomArg custom_arg = {
        .index = 1,
        .pause_check = false,
        .resume_check = false
    };
    CVI_PLAYER_SetCustomArgEventHandler(player, eventHandler, &custom_arg);
    CVI_PLAYER_Play(player);
    CVI_PLAYER_Pause(player);
    CVI_PLAYER_Resume(player);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_TRUE(custom_arg.pause_check);
    EXPECT_TRUE(custom_arg.resume_check);
    CVI_PLAYER_Stop(player);
    CVI_PLAYER_Destroy(&player);
    CVI_PLAYER_Deinit();
}

TEST(UnitTest, MutiplePlayerAndCustomArgEventTest) {
    struct CustomArg {
        int32_t index;
    };
    CVI_PLAYER_Init();
    // player 1
    CVI_PLAYER_HANDLE_T player1 = NULL;
    CVI_PLAYER_Create(&player1);
    CVI_PLAYER_SetDataSource(player1, data_source);
    auto eventHandler1 = [] (void *arg, CVI_PLAYER_HANDLE_T player, CVI_PLAYER_EVENT_S *event) {
        CustomArg *custom_arg = static_cast<CustomArg *>(arg);
        EXPECT_EQ(custom_arg->index, 1);
    };
    CustomArg custom_arg1 = {
        .index = 1
    };
    CVI_PLAYER_SetCustomArgEventHandler(player1, eventHandler1, &custom_arg1);
    // player 2
    CVI_PLAYER_HANDLE_T player2 = NULL;
    CVI_PLAYER_Create(&player2);
    CVI_PLAYER_SetDataSource(player2, data_source);
    auto eventHandler2 = [] (void *arg, CVI_PLAYER_HANDLE_T player, CVI_PLAYER_EVENT_S *event) {
        CustomArg *custom_arg = static_cast<CustomArg *>(arg);
        EXPECT_EQ(custom_arg->index, 2);
    };
    CustomArg custom_arg2 = {
        .index = 2
    };
    CVI_PLAYER_SetCustomArgEventHandler(player2, eventHandler2, &custom_arg2);

    CVI_PLAYER_Play(player1);
    CVI_PLAYER_Play(player2);
    CVI_PLAYER_Pause(player1);
    CVI_PLAYER_Pause(player2);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CVI_PLAYER_Stop(player1);
    CVI_PLAYER_Stop(player2);
    CVI_PLAYER_Destroy(&player1);
    CVI_PLAYER_Destroy(&player2);
    CVI_PLAYER_Deinit();
}

TEST(UnitTest, ExtraPacketTest) {
    CVI_PLAYER_Init();
    CVI_PLAYER_HANDLE_T player = NULL;
    CVI_PLAYER_Create(&player);
    CVI_PLAYER_SetDataSource(player, data_source);
    CVI_PLAYER_Play(player);
    CVI_PLAYER_PACKET_S extra_packet{};
    int32_t ret = CVI_PLAYER_GetVideoExtraPacket(player, &extra_packet);
    if (ret == 0) {
        EXPECT_NE(extra_packet.size, 0);
    }
    CVI_PLAYER_Stop(player);
    CVI_PLAYER_Destroy(&player);
    CVI_PLAYER_Deinit();
}

TEST(UnitTest, StressTest) {
    CVI_PLAYER_Init();
    constexpr int32_t loop_times = 10;
    CVI_PLAYER_HANDLE_T player = NULL;
    CVI_PLAYER_Create(&player);
    CVI_PLAYER_SetDataSource(player, data_source);
    for (int32_t i = 0; i < loop_times; ++i) {
        CVI_PLAYER_Play(player);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        CVI_PLAYER_PLAY_INFO play_info = {};
        CVI_PLAYER_GetPlayInfo(player, &play_info);
        EXPECT_NE(play_info.duration_sec, 0);
        CVI_PLAYER_Stop(player);
    }
    CVI_PLAYER_Destroy(&player);
    CVI_PLAYER_Deinit();
}

} // anonymous namespace
