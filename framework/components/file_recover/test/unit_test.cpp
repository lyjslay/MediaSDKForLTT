#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "cvi_file_recover/cvi_file_recover.h"

namespace {

constexpr char test_input[] = "test.mov";
constexpr char test_output[] = "repair.mov";

TEST(UnitTest, BaseTest) {
    int32_t ret = 0;
    CVI_FILE_RECOVER_HANDLE_T handle = nullptr;
    CVI_FILE_RECOVER_Create(&handle);
    ret = CVI_FILE_RECOVER_Open(handle, test_input);
    ASSERT_EQ(ret, 0);
    CVI_FILE_RECOVER_Dump(handle);
    ret = CVI_FILE_RECOVER_Check(handle);
    EXPECT_EQ(ret, -1);
    ret = CVI_FILE_RECOVER_Recover(handle, test_output);
    EXPECT_EQ(ret, 0);
    CVI_FILE_RECOVER_Dump(handle);
    ret = CVI_FILE_RECOVER_Check(handle);
    EXPECT_EQ(ret, 0);
    CVI_FILE_RECOVER_Destroy(&handle);
    EXPECT_EQ(handle, nullptr);
}

TEST(UnitTest, EventTest) {
    struct CustomArg {
        int32_t index{1};
        bool progress_check;
        bool start_check;
        bool finish_check;
    };
    CustomArg custom_arg = {};
    int32_t ret = 0;
    CVI_FILE_RECOVER_HANDLE_T handle = nullptr;
    CVI_FILE_RECOVER_Create(&handle);
    auto event_handler = [](void* arg, CVI_FILE_RECOVER_EVENT_S* event) {
        CustomArg* custom_arg = static_cast<CustomArg*>(arg);
        switch(event->type) {
        case CVI_FILE_RECOVER_EVENT_RECOVER_PROGRESS:
            EXPECT_EQ(custom_arg->index, 1);
            custom_arg->progress_check = true;
            break;
        case CVI_FILE_RECOVER_EVENT_RECOVER_START:
            EXPECT_EQ(custom_arg->index, 1);
            custom_arg->start_check = true;
            break;
        case CVI_FILE_RECOVER_EVENT_RECOVER_FINISHED:
            EXPECT_EQ(custom_arg->index, 1);
            custom_arg->finish_check = true;
            break;
        default:
            break;
        }
    };
    CVI_FILE_RECOVER_SetCustomArgEventHandler(handle, event_handler, &custom_arg);
    ret = CVI_FILE_RECOVER_Open(handle, test_input);
    ASSERT_EQ(ret, 0);
    ret = CVI_FILE_RECOVER_RecoverAsync(handle, test_output);
    EXPECT_EQ(ret, 0);
    CVI_FILE_RECOVER_RecoverJoin(handle);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_TRUE(custom_arg.start_check);
    EXPECT_TRUE(custom_arg.progress_check);
    EXPECT_TRUE(custom_arg.finish_check);
    CVI_FILE_RECOVER_Destroy(&handle);
    EXPECT_EQ(handle, nullptr);
}

} // anonymous namespace
