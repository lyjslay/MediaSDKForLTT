#pragma once

#include "cvi_mapi.h"

namespace cvi_image_viewer {

class DisplayHelper final
{
public:
    static inline int32_t send(CVI_MAPI_WND_HANDLE_T handle, VIDEO_FRAME_INFO_S &frame)
    {
        return CVI_MAPI_DISP_SendWndFrame(handle, &frame);
    }

    static inline bool setAttributeIfNotEqual(CVI_MAPI_WND_HANDLE_T handle,
        const POINT_S &pos, const SIZE_S &size)
    {
        CVI_MAPI_WND_ATTR_T window_attr = {};
        if (CVI_MAPI_DISP_GetWndAttr(handle, &window_attr) != 0) {
            CVI_LOGE("MAPI get window attr failed");
            return false;
        }

        if ((window_attr.wnd_x == static_cast<uint32_t>(pos.s32X)) &&
            (window_attr.wnd_y == static_cast<uint32_t>(pos.s32Y)) &&
            (window_attr.wnd_w == size.u32Width) &&
            (window_attr.wnd_h == size.u32Height)) {
            return true;
        }

        window_attr.wnd_x = static_cast<uint32_t>(pos.s32X);
        window_attr.wnd_y = static_cast<uint32_t>(pos.s32Y);
        window_attr.wnd_w = size.u32Width;
        window_attr.wnd_h = size.u32Height;
        if (CVI_MAPI_DISP_SetWndAttr(handle, &window_attr) != 0) {
            CVI_LOGE("MAPI set window attr failed");
            return false;
        }

        return true;
    }
};

} // namespace cvi_image_viewer
