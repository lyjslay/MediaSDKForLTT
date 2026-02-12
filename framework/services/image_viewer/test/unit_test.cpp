#include <gtest/gtest.h>
#include <chrono>
#include <string>
#include <thread>
#include "cvi_image_viewer/cvi_image_viewer.h"
#include "cvi_mapi_sys.h"
#include "cvi_comm_video.h"

namespace {

std::vector<std::string> test_input = {"test.mov", "test2.mov"};

class UnitTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        CVI_MAPI_MEDIA_SYS_ATTR_T sys_attr;
        sys_attr.vb_pool[0].is_frame = true;
        // decoder, JPEG max size is 3840 * 2160
        sys_attr.vb_pool[0].vb_blk_size.frame.width = JPEGD_MAX_WIDTH;
        sys_attr.vb_pool[0].vb_blk_size.frame.height = JPEGD_MAX_HEIGHT;
        sys_attr.vb_pool[0].vb_blk_size.frame.fmt = PIXEL_FORMAT_YUV_PLANAR_420;
        sys_attr.vb_pool[0].vb_blk_num = 3;
        // vpss
        sys_attr.vb_pool[1].is_frame = true;
        sys_attr.vb_pool[1].vb_blk_size.frame.width = 1920;
        sys_attr.vb_pool[1].vb_blk_size.frame.height = 1080;
        sys_attr.vb_pool[1].vb_blk_size.frame.fmt = PIXEL_FORMAT_YUV_PLANAR_420;
        sys_attr.vb_pool[1].vb_blk_num = 3;
        // display
        sys_attr.vb_pool[2].is_frame = true;
        sys_attr.vb_pool[2].vb_blk_size.frame.width = 1280;
        sys_attr.vb_pool[2].vb_blk_size.frame.height = 720;
        sys_attr.vb_pool[2].vb_blk_size.frame.fmt = PIXEL_FORMAT_YUV_PLANAR_420;
        sys_attr.vb_pool[2].vb_blk_num = 18;
        sys_attr.vb_pool_num = 3;
        CVI_MAPI_Media_Init(&sys_attr);
    }

    static void TearDownTestSuite() {
        CVI_MAPI_Media_Deinit();
    }

    struct CustomArg {
        int32_t index;
    };
};

TEST_F(UnitTest, BaseTest) {
    int32_t ret = 0;
    CVI_IMAGE_VIEWER_HANDLE_T viewer_handle = nullptr;
    ret = CVI_IMAGE_VIEWER_Create(&viewer_handle);
    ASSERT_EQ(ret, 0);
    POINT_S pos = {
        .s32X = 100,
        .s32Y = 200
    };
    SIZE_S size = {
        .u32Width = 200,
        .u32Height = 100
    };
    ret = CVI_IMAGE_VIEWER_DisplayThumbnail(viewer_handle, test_input[0].c_str(), pos, size);
    EXPECT_EQ(ret, 0);
    if (ret == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    CVI_IMAGE_VIEWER_Destroy(&viewer_handle);
    EXPECT_EQ(viewer_handle, nullptr);
}

TEST_F(UnitTest, MutipleDisplayTest) {
    int32_t ret = 0;
    CVI_IMAGE_VIEWER_HANDLE_T viewer_handle = nullptr;
    ret = CVI_IMAGE_VIEWER_Create(&viewer_handle);
    ASSERT_EQ(ret, 0);
    POINT_S pos = {
        .s32X = 100,
        .s32Y = 200
    };
    SIZE_S size = {
        .u32Width = 200,
        .u32Height = 100
    };
    ret = CVI_IMAGE_VIEWER_DisplayThumbnail(viewer_handle, test_input[0].c_str(), pos, size);
    EXPECT_EQ(ret, 0);
    if (ret == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    pos.s32X = 300;
    pos.s32Y = 300;
    size.u32Width = 300;
    size.u32Height = 200;
    ret = CVI_IMAGE_VIEWER_DisplayThumbnail(viewer_handle, test_input[1].c_str(), pos, size);
    EXPECT_EQ(ret, 0);
    if (ret == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    CVI_IMAGE_VIEWER_Destroy(&viewer_handle);
    EXPECT_EQ(viewer_handle, nullptr);
}

TEST_F(UnitTest, MutipleHandleTest) {
    int32_t ret = 0;
    CVI_IMAGE_VIEWER_HANDLE_T viewer_handle1 = nullptr;
    CVI_IMAGE_VIEWER_HANDLE_T viewer_handle2 = nullptr;
    ret = CVI_IMAGE_VIEWER_Create(&viewer_handle1);
    ASSERT_EQ(ret, 0);
    ret = CVI_IMAGE_VIEWER_Create(&viewer_handle2);
    ASSERT_EQ(ret, 0);
    POINT_S pos = {
        .s32X = 100,
        .s32Y = 200
    };
    SIZE_S size = {
        .u32Width = 200,
        .u32Height = 100
    };
    ret = CVI_IMAGE_VIEWER_DisplayThumbnail(viewer_handle1, test_input[0].c_str(), pos, size);
    EXPECT_EQ(ret, 0);
    pos.s32X = 400;
    pos.s32Y = 300;
    size.u32Width = 300;
    size.u32Height = 200;
    ret = CVI_IMAGE_VIEWER_DisplayThumbnail(viewer_handle2, test_input[1].c_str(), pos, size);
    EXPECT_EQ(ret, 0);
    if (ret == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    CVI_IMAGE_VIEWER_Destroy(&viewer_handle1);
    EXPECT_EQ(viewer_handle1, nullptr);
    CVI_IMAGE_VIEWER_Destroy(&viewer_handle2);
    EXPECT_EQ(viewer_handle2, nullptr);
}

TEST_F(UnitTest, HandlerTest) {
    int32_t ret = 0;
    CVI_IMAGE_VIEWER_HANDLE_T viewer_handle = nullptr;
    ret = CVI_IMAGE_VIEWER_Create(&viewer_handle);
    ASSERT_EQ(ret, 0);
    CustomArg custom_arg = {
        .index = 1
    };
    auto handler = [](void *arg, VIDEO_FRAME_INFO_S *frame) {
        CustomArg *custom_arg = static_cast<CustomArg *>(arg);
        EXPECT_EQ(custom_arg->index, 1);
    };
    ret = CVI_IMAGE_VIEWER_SetCustomArgOutputHandler(viewer_handle, handler, &custom_arg);
    EXPECT_EQ(ret, 0);
    POINT_S pos = {
        .s32X = 100,
        .s32Y = 200
    };
    SIZE_S size = {
        .u32Width = 200,
        .u32Height = 100
    };
    ret = CVI_IMAGE_VIEWER_DisplayThumbnail(viewer_handle, test_input[0].c_str(), pos, size);
    EXPECT_EQ(ret, 0);
    if (ret == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    CVI_IMAGE_VIEWER_Destroy(&viewer_handle);
    EXPECT_EQ(viewer_handle, nullptr);
}

} // anonymous namespace
