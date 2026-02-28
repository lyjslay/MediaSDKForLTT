#include <gtest/gtest.h>
#include <stdint.h>
#include "cvi_demuxer/cvi_demuxer.h"

namespace {

constexpr char test_input[] = "test.mov";

TEST(UnitTest, BaseTest) {
    CVI_DEMUXER_PACKET_S packet = {};
    int64_t prev_pts = 0;
    constexpr int32_t pts_diff = 512;
    CVI_DEMUXER_HANDLE_T demuxer = NULL;
    CVI_DEMUXER_Create(&demuxer);
    CVI_DEMUXER_SetInput(demuxer, test_input);
    int32_t ret = CVI_DEMUXER_Open(demuxer);
    ASSERT_EQ(ret, 0);

    // test get media info
    CVI_DEMUXER_MEDIA_INFO_S info = {};
    ret = CVI_DEMUXER_GetMediaInfo(demuxer, &info);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(info.width, 0);
    EXPECT_NE(info.height, 0);
    EXPECT_NE(info.file_size, 0);

    // test read
    ret = CVI_DEMUXER_Read(demuxer, &packet);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(packet.pts, 0);
    EXPECT_NE(packet.size, 0);
    prev_pts = packet.pts;
    ret = CVI_DEMUXER_Read(demuxer, &packet);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(prev_pts, packet.pts);

    // test seek
    CVI_DEMUXER_Seek(demuxer, 500);
    prev_pts = packet.pts;
    ret = CVI_DEMUXER_Read(demuxer, &packet);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(prev_pts + pts_diff, packet.pts);

    // test pause/resume
    CVI_DEMUXER_Pause(demuxer);
    prev_pts = packet.pts;
    ret = CVI_DEMUXER_Read(demuxer, &packet);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(prev_pts, packet.pts);
    CVI_DEMUXER_Resume(demuxer);
    prev_pts = packet.pts;
    ret = CVI_DEMUXER_Read(demuxer, &packet);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(prev_pts, packet.pts);

    CVI_DEMUXER_Close(demuxer);
    CVI_DEMUXER_Destroy(&demuxer);
    EXPECT_EQ(demuxer, nullptr);
}

} // anonymous namespace
