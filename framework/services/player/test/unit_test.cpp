#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "cvi_player_service.h"
#include "cvi_mapi_sys.h"
#include "cvi_player/event/event.h"
#include "cvi_player/cvi_player.h"
#include "cvi_signal_slot/cvi_signal_slot.h"

namespace {

std::vector<std::string> test_input = {"test.mov"};

class UnitTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        CVI_MAPI_MEDIA_SYS_ATTR_T sys_attr;
        sys_attr.vb_pool[0].is_frame = true;
        sys_attr.vb_pool[0].vb_blk_size.frame.width = 2560;
        sys_attr.vb_pool[0].vb_blk_size.frame.height = 1440;
        sys_attr.vb_pool[0].vb_blk_size.frame.fmt = PIXEL_FORMAT_YUV_PLANAR_420;
        sys_attr.vb_pool[0].vb_blk_num = 9;
        // display
        sys_attr.vb_pool[1].is_frame = true;
        sys_attr.vb_pool[1].vb_blk_size.frame.width = 1280;
        sys_attr.vb_pool[1].vb_blk_size.frame.height = 720;
        sys_attr.vb_pool[1].vb_blk_size.frame.fmt = PIXEL_FORMAT_NV21;
        sys_attr.vb_pool[1].vb_blk_num = 18;
        sys_attr.vb_pool_num = 2;
        CVI_MAPI_Media_Init(&sys_attr);
    }

    static void TearDownTestSuite() {
        CVI_MAPI_Media_Deinit();
    }
};

TEST_F(UnitTest, BaseTest) {
    CVI_PLAYER_SERVICE_HANDLE_T handle = nullptr;
    int32_t ret = CVI_PLAYER_SERVICE_Create(&handle, nullptr);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_SetInput(handle, test_input[0].c_str());
    ASSERT_EQ(ret, 0);
    CVI_PLAYER_MEDIA_INFO_S info = {};
    ret = CVI_PLAYER_SERVICE_GetMediaInfo(handle, &info);
    ASSERT_EQ(ret, 0);
    EXPECT_NE(info.width, 0);
    EXPECT_NE(info.height, 0);
    EXPECT_NE(info.file_size, 0);
    ret = CVI_PLAYER_SERVICE_Play(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ret = CVI_PLAYER_SERVICE_Pause(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ret = CVI_PLAYER_SERVICE_Play(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    CVI_PLAYER_SERVICE_Stop(handle);
    CVI_PLAYER_SERVICE_Destroy(&handle);
    EXPECT_EQ(handle, nullptr);
}

TEST_F(UnitTest, ResizeAndMoveTest) {
    CVI_PLAYER_SERVICE_HANDLE_T handle = nullptr;
    CVI_PLAYER_SERVICE_PARAM_S param = {};
    CVI_PLAYER_SERVICE_GetDefaultParam(&param);
    param.disp_aspect_ratio = ASPECT_RATIO_MANUAL;
    param.x = 100;
    param.y = 100;
    param.width = 500;
    param.height = 400;
    int32_t ret = CVI_PLAYER_SERVICE_Create(&handle, &param);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_SetInput(handle, test_input[0].c_str());
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_Play(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ret = CVI_PLAYER_SERVICE_MoveTo(handle, 0, 0);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ret = CVI_PLAYER_SERVICE_Resize(handle, 640, 480);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ret = CVI_PLAYER_SERVICE_ToggleFullscreen(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ret = CVI_PLAYER_SERVICE_ToggleFullscreen(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    CVI_PLAYER_SERVICE_Stop(handle);
    CVI_PLAYER_SERVICE_Destroy(&handle);
}

TEST_F(UnitTest, DestroyTest) {
    int32_t ret = 0;
    CVI_PLAYER_SERVICE_HANDLE_T handle = nullptr;
    ret = CVI_PLAYER_SERVICE_Create(&handle, nullptr);
    ASSERT_EQ(ret, 0);
    CVI_PLAYER_SERVICE_Destroy(&handle);
}

TEST_F(UnitTest, SeekTest) {
    CVI_PLAYER_SERVICE_HANDLE_T handle = nullptr;
    int32_t ret = CVI_PLAYER_SERVICE_Create(&handle, nullptr);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_SetInput(handle, test_input[0].c_str());
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_Play(handle);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_Seek(handle, 2000);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    CVI_PLAYER_SERVICE_Stop(handle);
    CVI_PLAYER_SERVICE_Destroy(&handle);
}

TEST_F(UnitTest, EventTest) {
    static bool event_play_check = false;
    static bool event_play_progress_check = false;
    static bool event_pause_check = false;
    static bool event_resume_check = false;

    auto event_handler = [](CVI_PLAYER_SERVICE_HANDLE_T handle, CVI_PLAYER_SERVICE_EVENT_S *event) {
        switch(event->type) {
        case CVI_PLAYER_SERVICE_EVENT_PLAY:
            event_play_check = true;
            break;
        case CVI_PLAYER_SERVICE_EVENT_PLAY_PROGRESS:
            {
                event_play_progress_check = true;
                CVI_PLAYER_PLAY_INFO info = {};
                int32_t ret = CVI_PLAYER_SERVICE_GetPlayInfo(handle, &info);
                EXPECT_EQ(ret, 0);
            }
            break;
        case CVI_PLAYER_SERVICE_EVENT_PAUSE:
            event_pause_check = true;
            break;
        case CVI_PLAYER_SERVICE_EVENT_RESUME:
            event_resume_check = true;
            break;
        default:
            break;
        }
    };

    CVI_PLAYER_SERVICE_HANDLE_T handle = nullptr;
    int32_t ret = CVI_PLAYER_SERVICE_Create(&handle, nullptr);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_SetInput(handle, test_input[0].c_str());
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_SetEventHandler(handle, event_handler);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_Play(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ret = CVI_PLAYER_SERVICE_Pause(handle);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_Play(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(event_play_check);
    EXPECT_TRUE(event_play_progress_check);
    EXPECT_TRUE(event_pause_check);
    EXPECT_TRUE(event_resume_check);
    CVI_PLAYER_SERVICE_Stop(handle);
    CVI_PLAYER_SERVICE_Destroy(&handle);
}

TEST_F(UnitTest, AoHandleTest) {
    int32_t ret = 0;
    CVI_MAPI_AO_HANDLE_T ao_handle = nullptr;
    ret = CVI_MAPI_AO_GetHandle(&ao_handle);
    if (ret != 0) {
        CVI_MAPI_AO_ATTR_S ao_attr = {};
        ao_attr.enSampleRate = AUDIO_SAMPLE_RATE_16000;
        ao_attr.channels = 1;
        ao_attr.u32PtNumPerFrm = 640;
        ret = CVI_MAPI_AO_Init(&ao_handle, &ao_attr);
        ASSERT_EQ(ret, 0);
    }

    CVI_PLAYER_SERVICE_HANDLE_T handle = nullptr;
    CVI_PLAYER_SERVICE_PARAM_S param = {};
    CVI_PLAYER_SERVICE_GetDefaultParam(&param);
    param.ao = ao_handle;

    ret = CVI_PLAYER_SERVICE_Create(&handle, &param);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_SetInput(handle, test_input[0].c_str());
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_Play(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    CVI_PLAYER_SERVICE_Stop(handle);
    CVI_PLAYER_SERVICE_Destroy(&handle);
}

TEST_F(UnitTest, StressTest) {
    constexpr int32_t count = 10;
    CVI_PLAYER_SERVICE_HANDLE_T handle = nullptr;
    int32_t ret = CVI_PLAYER_SERVICE_Create(&handle, nullptr);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_SetInput(handle, test_input[0].c_str());
    ASSERT_EQ(ret, 0);
    for (int32_t i = 0; i < count; ++i) {
        ret = CVI_PLAYER_SERVICE_Play(handle);
        ASSERT_EQ(ret, 0);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        CVI_PLAYER_SERVICE_Stop(handle);
    }
    CVI_PLAYER_SERVICE_Destroy(&handle);
}

static int32_t custom_play_handler(void *handle, CVI_PLAYER_MEDIA_INFO_S *info)
{
    int32_t *check = (int32_t *)handle;
    EXPECT_NE(info->width, 0);
    EXPECT_NE(info->height, 0);
    *check = 1;

    return 0;
}

TEST_F(UnitTest, SignalSlotTest) {
    CVI_PLAYER_SERVICE_HANDLE_T handle = nullptr;
    int32_t ret = CVI_PLAYER_SERVICE_Create(&handle, nullptr);
    ASSERT_EQ(ret, 0);
    // signals
    int32_t check = 0;
    CVI_PLAYER_SERVICE_SIGNALS_S *signals = nullptr;
    ret = CVI_PLAYER_SERVICE_GetSignals(handle, &signals);
    CVI_SLOT_S play_slot = {};
    CVI_SLOT_Init(&play_slot, &check, (CVI_INT_SLOT_VOID_HANLDER)custom_play_handler);
    CVI_SIGNAL_Connect(signals->play, play_slot);
    // slots
    CVI_PLAYER_SERVICE_SLOTS_S *slots = nullptr;
    ret = CVI_PLAYER_SERVICE_GetSlots(handle, &slots);
    CVI_SIGNAL_S custom_resize_signal = {};
    CVI_SIGNAL_InitByType(&custom_resize_signal, CVI_SIGNAL_SLOT_TYPE_UINT32_UINT32);
    CVI_SIGNAL_Connect(custom_resize_signal, slots->resize);
    CVI_SIGNAL_S toggle_fullscreen_signal = {};
    CVI_SIGNAL_InitByType(&toggle_fullscreen_signal, CVI_SIGNAL_SLOT_TYPE_NONE);
    CVI_SIGNAL_Connect(toggle_fullscreen_signal, slots->toggle_fullscreen);

    ret = CVI_PLAYER_SERVICE_SetInput(handle, test_input[0].c_str());
    ASSERT_EQ(ret, 0);
    CVI_PLAYER_MEDIA_INFO_S info = {};
    ret = CVI_PLAYER_SERVICE_GetMediaInfo(handle, &info);
    ASSERT_EQ(ret, 0);
    ret = CVI_PLAYER_SERVICE_Play(handle);
    ASSERT_EQ(ret, 0);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    CVI_SIGNAL_Emit(custom_resize_signal, 500, 400);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(check, 1);
    CVI_SIGNAL_Emit(toggle_fullscreen_signal);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    CVI_SIGNAL_Emit(toggle_fullscreen_signal);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    CVI_PLAYER_SERVICE_Stop(handle);
    CVI_PLAYER_SERVICE_Destroy(&handle);
    EXPECT_EQ(handle, nullptr);
    CVI_SLOT_Deinit(&play_slot);
    CVI_SIGNAL_Deinit(&custom_resize_signal);
    CVI_SIGNAL_Deinit(&toggle_fullscreen_signal);
}

} // anonymous namespace
