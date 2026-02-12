#ifndef __CVI_ADASMNG_H__
#define __CVI_ADASMNG_H__

#include <stdint.h>
#include "cvi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define CVI_ADASMNG_EREGISTER_EVENT CVI_APPCOMM_ERR_ID(CVI_APP_MOD_ADASMNG, CVI_ERRNO_CUSTOM_BOTTOM)

/** event ID define */
typedef enum cviEVENT_ADASMNG_E {
    CVI_EVENT_ADASMNG_CAR_MOVING = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_ADASMNG, 0),
    CVI_EVENT_ADASMNG_CAR_CLOSING,
    CVI_EVENT_ADASMNG_CAR_COLLISION,
    CVI_EVENT_ADASMNG_CAR_LANE,
    CVI_EVENT_ADASMNG_LABEL_CAR,
    CVI_EVENT_ADASMNG_LABEL_LANE,
    CVI_EVENT_ADASMNG_BUTT
} CVI_EVENT_ADASMNG_E;

typedef enum _CVI_ADASMNG_CMD_E
{
    CVI_ADASMNG_NORMAL = 0,
    CVI_ADASMNG_CAR_MOVING,
    CVI_ADASMNG_CAR_CLOSING,
    CVI_ADASMNG_CAR_COLLISION,
    CVI_ADASMNG_CAR_LANE,
    CVI_ADASMNG_LABEL_CAR,
    CVI_ADASMNG_LABEL_LANE,
    CVI_ADASMNG_BUTT
} CVI_ADASMNG_CMD_E;

int32_t CVI_ADASMNG_RegisterEvent(void);
int32_t CVI_ADASMNG_VoiceCallback(int32_t index);
int32_t CVI_ADASMNG_LabelCallback(int32_t camid, int32_t index, uint32_t count, char* coordinates);
int32_t CVI_ADASMNG_LabelOSDCCallback(int32_t camid, uint32_t car_count, char* car_coordinates, uint32_t lane_count, char* lane_coordinates);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_ADASMNG_H__ */