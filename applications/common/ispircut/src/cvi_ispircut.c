#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <pthread.h>

#include "cvi_mapi.h"
#include "cvi_appcomm.h"
#include "cvi_comm_video.h"
#include "cvi_eventhub.h"
#include "cvi_media_init.h"
#include "cvi_ae.h"
#include "cvi_ispircut.h"
#include "cvi_hal_gpio.h"
#include "cvi_bin.h"
#include "cvi_isp.h"
#include "cvi_comm_isp.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct _ISP_IR_CTX_S{
    cvi_osal_task_handle_t task;
    pthread_mutex_t mutex;
    int32_t     run;
    int32_t     s32IRControlMode;
    int32_t     s32CamId;
    int32_t     s32IrCutA;
    int32_t     s32IrCutB;
    int32_t     s32LedIr;
    uint32_t    s16Normal2IrIsoThr;
    uint32_t    s16Ir2NormalIsoThr;

    bool        bInit;
    int32_t     s32Mode; // 0:daymode; 1:night mode
    char        DayBinPath[128];
    char        NightBinPath[128];
}CVI_ISP_IR_CTX_S;

#define CHECK_COUNT 5
#define IR_CUT_SWITCH_TO_NORMAL 0
#define IR_CUT_SWITCH_TO_IR     1
#define IR_CUT_CLOSE            0
#define IR_CUT_OPEN             1

static CVI_ISP_IR_CTX_S gstISPIRCtx[MAX_CAMERA_INSTANCES];
static enum CVI_BIN_SECTION_ID s_BinId[4] = {CVI_BIN_ID_ISP0, CVI_BIN_ID_ISP1, CVI_BIN_ID_ISP2, CVI_BIN_ID_ISP3};

static int32_t ISP_IRCut_Switch(int32_t cam_id, bool bEnable)
{
    CVI_GPIO_NUM_E IR_CUT_A, IR_CUT_B;
    IR_CUT_A = gstISPIRCtx[cam_id].s32IrCutA;
    IR_CUT_B = gstISPIRCtx[cam_id].s32IrCutB;

    if ((IR_CUT_A < CVI_GPIO_MIN) || (IR_CUT_B < CVI_GPIO_MIN)) {
        CVI_LOGE("use default value IR_CUT_A:%d or IR_CUT_B:%d\n", IR_CUT_A, IR_CUT_B);
        return -1;
    }
    if (!gstISPIRCtx[cam_id].bInit) {
        CVI_GPIO_Export(IR_CUT_A);
        CVI_GPIO_Direction_Output(IR_CUT_A);

        CVI_GPIO_Export(IR_CUT_B);
        CVI_GPIO_Direction_Output(IR_CUT_B);

        gstISPIRCtx[cam_id].bInit = true;
    }

    if (bEnable) {
        CVI_GPIO_Set_Value(IR_CUT_A, CVI_GPIO_VALUE_H);
        CVI_GPIO_Set_Value(IR_CUT_B, CVI_GPIO_VALUE_L);

        usleep(500*1000);
        CVI_GPIO_Set_Value(IR_CUT_A, CVI_GPIO_VALUE_L);
    } else {
        CVI_GPIO_Set_Value(IR_CUT_A, CVI_GPIO_VALUE_L);
        CVI_GPIO_Set_Value(IR_CUT_B, CVI_GPIO_VALUE_H);

        usleep(500*1000);
        CVI_GPIO_Set_Value(IR_CUT_B, CVI_GPIO_VALUE_L);
    }

    return 0;
}

static int32_t ISP_PQBin_Load(int32_t id, const char *pBinPath)
{
    int32_t ret = 0;
    FILE *fp = NULL;
    unsigned char *buf = NULL;
    uint64_t file_size;

    fp = fopen((const char *)pBinPath, "rb");
    if (fp == NULL) {
        CVI_LOGE("Can't find bin(%s)\n", pBinPath);
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    buf = (unsigned char *)malloc(file_size);
    if (buf == NULL) {
        CVI_LOGE("%s\n", "Allocae memory fail");
        fclose(fp);
        return -1;
    }

    fread(buf, file_size, 1, fp);

    if (fp != NULL) {
        fclose(fp);
    }

    if (s_BinId[id] >= CVI_BIN_ID_ISP0 && s_BinId[id] <= CVI_BIN_ID_ISP3) {
        ret = CVI_BIN_LoadParamFromBin(CVI_BIN_ID_HEADER, buf);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("CVI_BIN_ImportBinData error! ret:(0x%x)\n", ret);
            free(buf);
            return -1;
        }
    }
    ret = CVI_BIN_LoadParamFromBin(s_BinId[id], buf);
    if (s_BinId[id] == CVI_BIN_ID_ISP0) {
        ret = CVI_BIN_LoadParamFromBin(CVI_BIN_ID_VPSS, buf);
        ret = CVI_BIN_LoadParamFromBin(CVI_BIN_ID_VO, buf);
    }

    free(buf);

    return ret;
}

static int32_t ISPIR_SetMono(int32_t cam_id, bool en)
{
    int32_t ret = 0;
    ISP_MONO_ATTR_S stMonoAttr = {0};

    ret = CVI_ISP_GetMonoAttr(cam_id, &stMonoAttr);
    if (ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_ISP_GetMonoAttr error! ret:(0x%x)\n", ret);
        return -1;
    }

    stMonoAttr.Enable = en;
    ret = CVI_ISP_SetMonoAttr(cam_id, &stMonoAttr);
    if (ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_ISP_SetMonoAttr error! ret:(0x%x)\n", ret);
        return -1;
    }

    return 1;
}

/*
 * @brief select ir-cut mode between auto and manual
 * value = 0; AutoCtl
 * value = 1; bManualCtrl
 */
void CVI_ISPIR_ControlModeSelect(int32_t cam_id, int32_t value)
{
    CVI_LOGI("cam_id:%d, IRCut control (%d)\n", cam_id, value);
    pthread_mutex_lock(&gstISPIRCtx[cam_id].mutex);
    gstISPIRCtx[cam_id].s32Mode = value;
    pthread_mutex_unlock(&gstISPIRCtx[cam_id].mutex);
}

void CVI_ISPIR_ManualCtrl(int32_t cam_id, int32_t state)
{
    char *DayBinPath = gstISPIRCtx[cam_id].DayBinPath;
    char *NightBinPath = gstISPIRCtx[cam_id].NightBinPath;
    CVI_GPIO_NUM_E s32LedIr = gstISPIRCtx[cam_id].s32LedIr;

    if (gstISPIRCtx[cam_id].s32IRControlMode == 1) {
        if (state == IR_CUT_SWITCH_TO_NORMAL) {
            CVI_LOGI("IRCut Manual control and switch to normal mode\n");
            /* open ir filter*/
            ISP_IRCut_Switch(cam_id, IR_CUT_OPEN);
            /* close ir-led */
            CVI_GPIO_Set_Value(s32LedIr, CVI_GPIO_VALUE_L);
            ISPIR_SetMono(cam_id, 0);
            cvi_osal_task_sleep(1000 * 1000);
            /* load PQ parameter */
            ISP_PQBin_Load(cam_id, DayBinPath);
            gstISPIRCtx[cam_id].s32Mode = 0;
        } if (state == IR_CUT_SWITCH_TO_IR) {
            CVI_LOGI("IRCut Manual control and switch to IR mode\n");
            /* load PQ parameter */
            ISPIR_SetMono(cam_id, 1);
            ISP_PQBin_Load(cam_id, NightBinPath);
            cvi_osal_task_sleep(2000 * 1000);
            /* close ir filter*/
            ISP_IRCut_Switch(cam_id, IR_CUT_CLOSE);
            /* open ir-led */
            CVI_GPIO_Set_Value(s32LedIr, CVI_GPIO_VALUE_H);
            gstISPIRCtx[cam_id].s32Mode = 1;
        }
    } else {
        CVI_LOGE("for IRCutMode_ManualCtrl, s32IRControlMode must be 1, not %d\n", gstISPIRCtx[cam_id].s32IRControlMode);
    }
}

static void ThreadIRCutAutoSwitch(void *arg)
{
    CVI_ISP_IR_CTX_S *pstCtx = (CVI_ISP_IR_CTX_S *)arg;

    int16_t lv;
    uint8_t checkDayCount = 0;
    uint8_t checkNightCount = 0;
    int16_t ENTER_DAY_LV_LEVEL = pstCtx->s16Ir2NormalIsoThr;
    int16_t ENTER_NIGHT_LV_LEVEL = pstCtx->s16Normal2IrIsoThr;
    int32_t cam_id = pstCtx->s32CamId;

    CVI_GPIO_NUM_E IR_LED = pstCtx->s32LedIr;
    char *DayBinPath = pstCtx->DayBinPath;
    char *NightBinPath = pstCtx->NightBinPath;
    while(pstCtx->run)
    {
        if (pstCtx->s32IRControlMode == 0) {
            CVI_ISP_GetCurrentLvX100(pstCtx->s32CamId, &lv);
            CVI_LOGD("IRCut Auto control lv is %d, Day_LV=%d Night_LV=%d\n", lv, ENTER_DAY_LV_LEVEL, ENTER_NIGHT_LV_LEVEL);
            if (lv > ENTER_DAY_LV_LEVEL) {
                if (checkDayCount < CHECK_COUNT) {
                    checkDayCount++;
                } else if(pstCtx->s32Mode != 0) {
                    CVI_LOGD("IRCut Manual control and switch to normal mode\n");
                    /* open ir filter*/
                    ISP_IRCut_Switch(cam_id, IR_CUT_OPEN);
                    /* close ir-led */
                    CVI_GPIO_Set_Value(IR_LED, CVI_GPIO_VALUE_L);
                    ISPIR_SetMono(cam_id, 0);
                    /* load PQ parameter */
                    ISP_PQBin_Load(cam_id, DayBinPath);

                    pstCtx->s32Mode = 0;
                    checkDayCount = 0;
                }
            } else if (lv < ENTER_NIGHT_LV_LEVEL) {
                if (checkNightCount < CHECK_COUNT) {
                    checkNightCount++;
                } else if(pstCtx->s32Mode != 1) {
                    CVI_LOGD("IRCut Manual control and switch to IR mode\n");
                    /* load PQ parameter */
                    ISPIR_SetMono(cam_id, 0);
                    ISP_PQBin_Load(cam_id, NightBinPath);
                    cvi_osal_task_sleep(1000 * 1000);
                    /* close ir filter*/
                    ISP_IRCut_Switch(cam_id, IR_CUT_CLOSE);
                    /* open ir-led */
                    CVI_GPIO_Set_Value(IR_LED, CVI_GPIO_VALUE_H);
                    pstCtx->s32Mode = 1;
                    checkNightCount = 0;
                }
            } else {
                checkDayCount = 0;
                checkNightCount = 0;
            }
        }
        cvi_osal_task_sleep(1000 * 1000);
    }
    CVI_LOGD("ThreadIRCutAutoSwitch  exit");
}

int32_t CVI_ISPIR_Init(CVI_PARAM_ISPIR_ATTR_S *ISPIR)
{
    memset(&gstISPIRCtx, 0x0, sizeof(CVI_ISP_IR_CTX_S)*MAX_CAMERA_INSTANCES);
    for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
        if (ISPIR->stIspIrChnAttrs[i].bEnable == false) {
            CVI_LOGD("cam%d: isp ir_cut disable\n", i);
            continue;
        }
        CVI_ISP_IR_CTX_S *IRCtx = &gstISPIRCtx[i];

        pthread_mutex_init(&IRCtx->mutex, NULL);
        IRCtx->run = 1;
        IRCtx->s32IRControlMode = ISPIR->stIspIrChnAttrs[i].s32IRControlMode;
        IRCtx->s32CamId = ISPIR->stIspIrChnAttrs[i].s32CamId;
        IRCtx->s32IrCutA = ISPIR->stIspIrChnAttrs[i].s32IrCutA;
        IRCtx->s32IrCutB = ISPIR->stIspIrChnAttrs[i].s32IrCutB;
        IRCtx->s32LedIr = ISPIR->stIspIrChnAttrs[i].s32LedIr;
        IRCtx->s16Normal2IrIsoThr = ISPIR->stIspIrChnAttrs[i].s16Normal2IrIsoThr;
        IRCtx->s16Ir2NormalIsoThr = ISPIR->stIspIrChnAttrs[i].s16Ir2NormalIsoThr;
        memcpy(IRCtx->DayBinPath, ISPIR->stIspIrChnAttrs[i].DayBinPath, sizeof(IRCtx->DayBinPath));
        memcpy(IRCtx->NightBinPath, ISPIR->stIspIrChnAttrs[i].NightBinPath, sizeof(IRCtx->NightBinPath));

        ISP_IRCut_Switch(IRCtx->s32CamId, IR_CUT_OPEN);

        cvi_osal_task_attr_t ta;
        char ta_name[16] = {0};
        snprintf(ta_name, sizeof(ta_name), "ISP_IR_Proc%d", i);
        ta.name = ta_name;
        ta.entry = ThreadIRCutAutoSwitch;
        ta.param = (void *)IRCtx;
        ta.priority = CVI_OSAL_PRI_NORMAL;
        ta.detached = false;
        int32_t rc = cvi_osal_task_create(&ta, &IRCtx->task);
        if (rc != 0) {
            CVI_LOGE("rs_state task create failed, %d", rc);
            return -1;
        }

    }

    return 0;
}

int32_t CVI_ISPIR_DeInit(void)
{
    for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
        if (gstISPIRCtx[i].bInit == 0) {
            continue;
        }
        gstISPIRCtx[i].run = 0;
        cvi_osal_task_join(gstISPIRCtx[i].task);
        cvi_osal_task_destroy(&gstISPIRCtx[i].task);
        pthread_mutex_destroy(&gstISPIRCtx[i].mutex);

        CVI_GPIO_NUM_E IR_LED = gstISPIRCtx[i].s32LedIr;
        CVI_GPIO_NUM_E IR_CUT_A = gstISPIRCtx[i].s32IrCutA;
        CVI_GPIO_NUM_E IR_CUT_B = gstISPIRCtx[i].s32IrCutB;
        CVI_GPIO_Set_Value(IR_CUT_A, CVI_GPIO_VALUE_L);
        CVI_GPIO_Set_Value(IR_CUT_B, CVI_GPIO_VALUE_L);
        CVI_GPIO_Set_Value(IR_LED, CVI_GPIO_VALUE_L);
        CVI_GPIO_Unexport(IR_CUT_A);
        CVI_GPIO_Unexport(IR_CUT_B);
        CVI_GPIO_Unexport(IR_LED);

        gstISPIRCtx[i].bInit = 0;
    }

    CVI_LOGD("state detect exit\n");

    return 0;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */