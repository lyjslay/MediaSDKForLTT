#include "common_yocsystem.h"
#include <stdbool.h>
#include <aos/kv.h>
#include <debug/dbg.h>
#include <aos/cli.h>
#include <uservice/uservice.h>
#include <yoc/partition.h>
#include <yoc/init.h>
#include <drv/dma.h>
#include "board.h"
#include "debug/debug_cli_cmd.h"
#define AUDIO_DMA_ENABLE 0

#if AUDIO_DMA_ENABLE
static csi_dma_t dma;
#endif

static void stduart_init(void)
{
#ifdef ENABLE_UART_DEBUG_ON
    extern void console_init(int idx, uint32_t baud, uint16_t buf_size);
    console_init(UART_INDEX, UART_BAUD, 512);
    printf("##console_init## %d %d\n", UART_INDEX, UART_BAUD);
#endif
}

extern void  cxx_system_init(void);
void YOC_SYSTEM_Init(void)
{
    cxx_system_init();
    board_init();
    stduart_init();
    printf("###YoC###[%s,%s]\n", __DATE__, __TIME__);
    //printf("cpu clock is %dHz\n", soc_get_cpu_freq(0));
    #if AUDIO_DMA_ENABLE
    csi_dma_init(&dma, 0);
    #endif
}

// extern void cli_reg_cmd_ps(void);

void YOC_SYSTEM_ToolInit()
{
    //CLI放后面注册 先起流媒体
#ifdef ENABLE_UART_DEBUG_ON
    aos_cli_init();
    debug_cli_cmd_init();
#endif
    // cli_reg_cmd_ps();
    // ulog_init();
    // aos_set_log_level(AOS_LL_INFO);
    // event_service_init(NULL);
}

void cli_dump_isp_param(int argc,char **argv)
{
    char * pIspString = getenv("ISPPQPARAM");
    if(pIspString) {
        printf("********************************\n");
        printf("%s\n",pIspString);
        printf("********************************\n");
    } else {
        printf("%s pIspString is null\n",__func__);
    }
}
ALIOS_CLI_CMD_REGISTER(cli_dump_isp_param,dump_isp_pqparm,dump_isp_pqparm);
