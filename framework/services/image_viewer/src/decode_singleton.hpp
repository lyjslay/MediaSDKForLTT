#pragma once

#include "singleton.hpp"
#include "cvi_mapi_vdec.h"
#include <stdexcept>

namespace cvi_image_viewer {

class DecodeSingleton final : private Singleton<DecodeSingleton>
{
public:
    friend class Singleton<DecodeSingleton>;

    static const CVI_MAPI_VDEC_HANDLE_T getHandle()
    {
        return getInstance().vdec_handle;
    }

private:
    DecodeSingleton()
    {
        CVI_MAPI_VDEC_CHN_ATTR_T vdec_attr;
        vdec_attr.codec = CVI_MAPI_VCODEC_JPG;
        vdec_attr.max_width = JPEGD_MAX_WIDTH;
        vdec_attr.max_height = JPEGD_MAX_HEIGHT;
        if (CVI_MAPI_VDEC_InitChn(&vdec_handle, &vdec_attr) != 0) {
            throw std::runtime_error("MAPI vdec init failed");
        }
    }

    virtual ~DecodeSingleton()
    {
        /// TODO: call CVI_MAPI_VDEC_DeinitChn will segmentation fault
        // CVI_MAPI_VDEC_DeinitChn(vdec_handle);
    }

private:
    CVI_MAPI_VDEC_HANDLE_T vdec_handle{nullptr};
};

} // namespace cvi_image_viewer
