#pragma once

#include "cvi_demuxer/packet.h"
#include "cvi_mapi_vdec.h"

namespace cvi_image_viewer {

class DecodeHelper final
{
public:
    static inline int32_t send(CVI_MAPI_VDEC_HANDLE_T handle, const CviDemuxerPacket &packet)
    {
        VDEC_STREAM_S stream = {};
        stream.u64PTS = packet.pts;
        stream.pu8Addr = packet.data;
        stream.u32Len = packet.size;
        stream.bEndOfFrame = true;
        stream.bEndOfStream = true;

        return CVI_MAPI_VDEC_SendStream(handle, &stream);
    }

    static inline int32_t get(CVI_MAPI_VDEC_HANDLE_T handle, VIDEO_FRAME_INFO_S &frame)
    {
        return CVI_MAPI_VDEC_GetFrame(handle, &frame);
    }

    static inline int32_t release(CVI_MAPI_VDEC_HANDLE_T handle, VIDEO_FRAME_INFO_S &frame)
    {
        return CVI_MAPI_VDEC_ReleaseFrame(handle, &frame);
    }
};

} // namespace cvi_image_viewer
