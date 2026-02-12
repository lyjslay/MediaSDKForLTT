#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "cvi_liveviewmng.h"
#ifdef SERVICES_LIVEVIEW_ON
#include "cvi_liveview.h"
#endif
#include "cvi_cmdmng.h"

#ifndef SERVICES_LIVEVIEW_ON
#define CVI_CMD_CLIENT_ID_LIVEVIEW (0)
typedef enum cvi_cmd_liveview_e {
    CVI_CMD_LIVEVIEW_INVALID = 0,
    CVI_CMD_LIVEVIEW_SHUTDOWN,
    CVI_CMD_LIVEVIEW_SWITCH,
    CVI_CMD_LIVEVIEW_MOVEUP,
    CVI_CMD_LIVEVIEW_MOVEDOWN,
    CVI_CMD_LIVEVIEW_MIRROR,
    CVI_CMD_LIVEVIEW_FILP,
    CVI_CMD_LIVEVIEW_ADJUSTFOCUS,
    CVI_CMD_LIVEVIEW_MAX
} cvi_cmd_liveview_t;
#define CVI_CMD_CHANNEL_ID_LIVEVIEW(liveview_id) (0x00 + (liveview_id))
#endif


int32_t CVI_LIVEVIEWMNG_Switch(uint32_t val)
{
    CVI_CMDMNG_SendMqCmd(CVI_CMD_CLIENT_ID_LIVEVIEW, CVI_CMD_CHANNEL_ID_LIVEVIEW(0), CVI_CMD_LIVEVIEW_SWITCH, val);
    return 0;
}

int32_t CVI_LIVEVIEWMNG_AdjustFocus(int32_t wndid , char* ratio)
{
    //todo need ack
    CVI_CMDMNG_SendMqCmd_Str(CVI_CMD_CLIENT_ID_LIVEVIEW, CVI_CMD_CHANNEL_ID_LIVEVIEW(0), CVI_CMD_LIVEVIEW_ADJUSTFOCUS, wndid , ratio);
    return 0;
}

int32_t CVI_LIVEVIEWMNG_MoveUp(int32_t wndid)
{
    //todo need ack
    CVI_CMDMNG_SendMqCmd(CVI_CMD_CLIENT_ID_LIVEVIEW, CVI_CMD_CHANNEL_ID_LIVEVIEW(0), CVI_CMD_LIVEVIEW_MOVEUP, wndid);
    return 0;
}

int32_t CVI_LIVEVIEWMNG_MoveDown(int32_t wndid)
{
    //todo need ack
    CVI_CMDMNG_SendMqCmd(CVI_CMD_CLIENT_ID_LIVEVIEW, CVI_CMD_CHANNEL_ID_LIVEVIEW(0), CVI_CMD_LIVEVIEW_MOVEDOWN, wndid);
    return 0;
}

int32_t CVI_LIVEVIEWMNG_Mirror(uint32_t val)
{
    //todo need ack
    CVI_CMDMNG_SendMqCmd(CVI_CMD_CLIENT_ID_LIVEVIEW, CVI_CMD_CHANNEL_ID_LIVEVIEW(0), CVI_CMD_LIVEVIEW_MIRROR, val);
    return 0;
}

int32_t CVI_LIVEVIEWMNG_Filp(uint32_t val)
{
    //todo need ack
    CVI_CMDMNG_SendMqCmd(CVI_CMD_CLIENT_ID_LIVEVIEW, CVI_CMD_CHANNEL_ID_LIVEVIEW(0), CVI_CMD_LIVEVIEW_FILP, val);
    return 0;
}
