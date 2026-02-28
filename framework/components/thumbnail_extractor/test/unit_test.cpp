#include <gtest/gtest.h>
#include "cvi_thumbnail_extractor/cvi_thumbnail_extractor.h"

namespace {

constexpr char test_input[] = "test.mov";

TEST(UnitTest, BaseTest) {
    CVI_THUMBNAIL_EXTRACTOR_HANDLE_T extractor_handle = nullptr;
    CVI_THUMBNAIL_EXTRACTOR_Create(&extractor_handle);
    CVI_THUMBNAIL_PACKET_S thumbnail = {};
    int32_t ret = CVI_THUMBNAIL_EXTRACTOR_GetThumbnail(extractor_handle,
        test_input, &thumbnail);
    ASSERT_EQ(ret, 0);
    EXPECT_NE(thumbnail.size, 0);
    CVI_THUMBNAIL_EXTRACTOR_Destroy(&extractor_handle);
    EXPECT_EQ(extractor_handle, nullptr);
    CVI_THUMBNAIL_EXTRACTOR_ClearPacket(&thumbnail);
    EXPECT_EQ(thumbnail.size, 0);
}

TEST(UnitTest, NullTest) {
    CVI_THUMBNAIL_EXTRACTOR_HANDLE_T extractor_handle = nullptr;
    CVI_THUMBNAIL_EXTRACTOR_Create(&extractor_handle);
    CVI_THUMBNAIL_PACKET_S *thumbnail = nullptr;
    int32_t ret = CVI_THUMBNAIL_EXTRACTOR_GetThumbnail(extractor_handle,
        test_input, thumbnail);
    ASSERT_NE(ret, 0);
    CVI_THUMBNAIL_EXTRACTOR_Destroy(&extractor_handle);
    EXPECT_EQ(extractor_handle, nullptr);
    ret = CVI_THUMBNAIL_EXTRACTOR_Destroy(&extractor_handle);
    ASSERT_NE(ret, 0);
}

TEST(UnitTest, MultipleHandleTest) {
    // create
    CVI_THUMBNAIL_EXTRACTOR_HANDLE_T extractor_handle1 = nullptr;
    CVI_THUMBNAIL_EXTRACTOR_HANDLE_T extractor_handle2 = nullptr;
    CVI_THUMBNAIL_EXTRACTOR_HANDLE_T extractor_handle3 = nullptr;
    CVI_THUMBNAIL_EXTRACTOR_Create(&extractor_handle1);
    CVI_THUMBNAIL_EXTRACTOR_Create(&extractor_handle2);
    CVI_THUMBNAIL_EXTRACTOR_Create(&extractor_handle3);
    CVI_THUMBNAIL_PACKET_S thumbnail1 = {};
    CVI_THUMBNAIL_PACKET_S thumbnail2 = {};
    CVI_THUMBNAIL_PACKET_S thumbnail3 = {};
    // get thumbnails
    int32_t ret = CVI_THUMBNAIL_EXTRACTOR_GetThumbnail(extractor_handle1,
        test_input, &thumbnail1);
    ASSERT_EQ(ret, 0);
    EXPECT_NE(thumbnail1.size, 0);
    ret = CVI_THUMBNAIL_EXTRACTOR_GetThumbnail(extractor_handle2,
        test_input, &thumbnail2);
    ASSERT_EQ(ret, 0);
    EXPECT_NE(thumbnail2.size, 0);
    ret = CVI_THUMBNAIL_EXTRACTOR_GetThumbnail(extractor_handle3,
        test_input, &thumbnail3);
    ASSERT_EQ(ret, 0);
    EXPECT_NE(thumbnail3.size, 0);
    // destroy
    CVI_THUMBNAIL_EXTRACTOR_Destroy(&extractor_handle1);
    EXPECT_EQ(extractor_handle1, nullptr);
    CVI_THUMBNAIL_EXTRACTOR_Destroy(&extractor_handle2);
    EXPECT_EQ(extractor_handle2, nullptr);
    CVI_THUMBNAIL_EXTRACTOR_Destroy(&extractor_handle3);
    EXPECT_EQ(extractor_handle3, nullptr);
    CVI_THUMBNAIL_EXTRACTOR_ClearPacket(&thumbnail1);
    CVI_THUMBNAIL_EXTRACTOR_ClearPacket(&thumbnail2);
    CVI_THUMBNAIL_EXTRACTOR_ClearPacket(&thumbnail3);
}

} // anonymous namespace
