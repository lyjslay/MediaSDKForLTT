#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <ucontext.h>
#include <semaphore.h>
#include <malloc.h>
#include "cvi_system.h"
#include <sys/reboot.h>
#include "cvi_media_init.h"
#include "cvi_param.h"
#include "cvi_eventhub.h"
#include "cvi_mode.h"
#include "cvi_storagemng.h"
#include "cvi_recordmng.h"
#ifdef SERVICES_PHOTO_ON
#include "cvi_photomng.h"
#endif
#include "cvi_filemng_dtcf.h"
#include "cvi_filemng_comm.h"
#include "cvi_usbctrl.h"
#include "cvi_gaugemng.h"
#include "cvi_tempermng.h"
#include "cvi_netctrl.h"
#include "cvi_ledmng.h"
#include "cvi_msg_client.h"
#ifdef CONFIG_GSENSOR_ON
#include "cvi_gsensormng.h"
#endif
#ifdef SERVICES_LIVEVIEW_ON
#include "cvi_volmng.h"
#endif
#ifdef CONFIG_GPS_ON
#include "cvi_gpsmng.h"
#endif
#ifdef CONFIG_WIFI_ON
#include "cvi_wifimng.h"
#endif
#ifdef CONFIG_ADC_ON
#include "cvi_gaugemng.h"
#endif
#ifdef CONFIG_WATCHDOG_ON
#include "cvi_watchdogmng.h"
#include "cvi_hal_watchdog.h"
#endif
#ifdef SERVICES_SPEECH_ON
#include "cvi_speechmng.h"
#include "cvi_speech.h"
#endif
#ifdef SERVICES_PLAYER_ON
#include "cvi_playbackmng.h"
#include "cvi_player_service.h"
#endif
#ifdef SERVICES_ADAS_ON
#include "cvi_adasmng.h"
#endif

// #include "stacktrace.h"
// #include <mcheck.h>
// #define ENABLE_MTRACE 1

#ifdef ENABLE_ISP_PQ_TOOL
#include "cvi_media_dump.h"
#endif

#include "cvi_ipcm.h"

static sem_t s_PowerOffSem;/** power off semaphore */
static pthread_t s_DelayedThread;
static pthread_t s_DelayedThread_Ao;
static CVI_MODEMNG_EXIT_MODE_E s_ExitMode = CVI_MODEMNG_EXIT_MODE_BUTT;/** exit mode */

void Sample_HandleSig(CVI_S32 signo)
{
    sem_post(&s_PowerOffSem);
    CVI_LOGE("Sample_HandleSig signal number %d\n",signo);
    exit(signo);
}

void signal_process()
{
	signal(SIGINT, Sample_HandleSig);
	signal(SIGTERM, Sample_HandleSig);
	signal(SIGSEGV, Sample_HandleSig);
	signal(SIGABRT, Sample_HandleSig);
}

static int32_t CVI_ExitModeCallback(CVI_MODEMNG_EXIT_MODE_E enExitMode)
{
    s_ExitMode = enExitMode;/** exit mode */
    sem_post(&s_PowerOffSem);
    return 0;
}

int32_t CVI_ModuleDelayedStart()
{
    pthread_detach(pthread_self());
    int32_t s32Ret = 0;
#ifdef CONFIG_KEY_ON
    CVI_KEYMNG_CFG_S stKeyCfg;
    s32Ret = CVI_PARAM_GetKeyMngCfg(&stKeyCfg);
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Get key cfg");
    s32Ret = CVI_KEYMNG_Init(stKeyCfg);
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Key init");
#endif
#ifdef CONFIG_ADC_ON
    CVI_GAUGEMNG_CFG_S stGaugeCfg;
    s32Ret = CVI_PARAM_GetGaugeMngCfg(&stGaugeCfg);
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "get gauge cfg");
    s32Ret = CVI_GAUGEMNG_Init(&stGaugeCfg);
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "gauge init");
    s32Ret = CVI_GAUGEMNG_RegisterEvent();
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "gauge register event");
#endif
#ifdef CONFIG_LED_ON
    s32Ret = CVI_LEDMNG_Init();
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Led init");
#endif
    s32Ret = CVI_MODEMNG_TEST_MAIN_Create();
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Test mode");

#ifdef CONFIG_GPS_ON
    s32Ret = CVI_GPSMNG_Init();
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Gps init");

    s32Ret = CVI_GPSMNG_Start();
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Gps start");
#endif

#ifdef CONFIG_GSENSOR_ON
    CVI_GSENSORMNG_CFG_S GsensorParam;
    CVI_PARAM_GetGsensorParam(&GsensorParam);
    s32Ret = CVI_GSENSORMNG_Init(&GsensorParam);
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Gsensor init");
#endif

#ifdef CONFIG_WIFI_ON
    CVI_PARAM_WIFI_S WifiParam;
    s32Ret = CVI_PARAM_GetWifiParam(&WifiParam);

    if (true == WifiParam.Enable) {
        s32Ret |= CVI_WIFIMNG_Start(WifiParam.WifiCfg, WifiParam.WifiDefaultSsid);
    }
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Wifi Init");
#endif

#ifdef CONFIG_WATCHDOG_ON
    CVI_S32 s32FeedTime_s = 10;   /**10s periodly feed dog*/
    s32Ret = CVI_WATCHDOGMNG_Init(s32FeedTime_s);
    CVI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "WATCHDOGMNG init");
#endif

#ifdef ENABLE_ISP_PQ_TOOL
    cvi_system("/etc/uhubon.sh host");
    cvi_system("/etc/uhubon.sh device");
    cvi_system("/etc/run_usb.sh probe rndis");
    cvi_system("/etc/run_usb.sh start");
    cvi_system("ifconfig usb0 192.168.0.103 up netmask 255.255.255.0");
#endif

    pthread_exit(0);
    return s32Ret;
}

int32_t CVI_ModuleDelayedStartThread(void)
{
    int32_t s32Ret = 0;

    s32Ret = pthread_create(&s_DelayedThread, NULL, (void*)CVI_ModuleDelayedStart, NULL);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return s32Ret;
}

static int32_t CVI_ModuleAoStart()
{
    pthread_detach(pthread_self());

    int32_t s32Ret = 0;
    s32Ret = CVI_MEDIA_AoInit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    pthread_exit(0);
    return s32Ret;
}

int32_t CVI_ModuleAoStartThread(void)
{
    int32_t s32Ret = 0;

    s32Ret = pthread_create(&s_DelayedThread_Ao, NULL, (void*)CVI_ModuleAoStart, NULL);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return s32Ret;
}


static int32_t module_init(void)
{
    int32_t s32Ret = 0;
    return 0;
    //set log level
    // release set CVI_LOG_INFO
    CVI_LOG_SET_LEVEL(CVI_LOG_ERROR);
    CVI_LOGD("main app starting...\n");

	CVI_IPCM_Init();

	s32Ret = CVI_PARAM_Init();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = CVI_SYSTEM_SetDefaultDateTime();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	int32_t timeout_cnt = 0;
	while (timeout_cnt < 50) {
		uint32_t ready = 0;
		CVI_IPCM_GetRtosBootStatus(&ready);
		if (ready & (1 << IPCM_RTOS_BOOTLOGO_DONE)) {
			break;
		}
		timeout_cnt++;
		usleep(100 * 1000);
	}

	if (timeout_cnt == 50) {
		CVI_LOGI("boot logo timeout\n");
	}

	s32Ret = CVI_MSG_Init();
	CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = CVI_ModuleAoStartThread();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = CVI_EVENTHUB_Init();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    CVI_STORAGEMNG_RegisterEvent();
    CVI_FILEMNG_RegisterEvent();
    CVI_RECORDMNG_RegisterEvent();
#ifdef SERVICES_PHOTO_ON
    CVI_POHTOMNG_RegisterEvent();
#endif
#ifdef SERVICES_PLAYER_ON
    CVI_PLAYBACKMNG_RegisterEvent();
#endif
#ifdef SERVICES_ADAS_ON
    CVI_ADASMNG_RegisterEvent();
#endif
    CVI_MODEMNG_RegisterEvent();
    // CVI_USBCTRL_RegisterEvent();

    CVI_SYSTEM_STARTUP_SRC_E enStartupSrc;
    CVI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
    CVI_LOGD("=============================enStartupSrc = %d\n",enStartupSrc);
    if(enStartupSrc == CVI_SYSTEM_STARTUP_SRC_GSENSORWAKEUP) {
        CVI_MODEMNG_SetParkingRec(true);
    } else {
        CVI_MODEMNG_SetParkingRec(false);
    }

    #ifdef ENABLE_ISP_PQ_TOOL
    bool en = false;
    CVI_MEDIA_DUMP_GetSizeStatus(&en);
    CVI_PARAM_CFG_S sysparams;
    CVI_PARAM_GetParam(&sysparams);
    if (en == true) {
        for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
            sysparams.MediaComm.Rtsp.ChnAttrs[i].BindVencId = 0 + i * 4;
        }
    } else {
         for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
            sysparams.MediaComm.Rtsp.ChnAttrs[i].BindVencId = 1 + i * 4;
        }
    }
    CVI_PARAM_SetParam(&sysparams);
    #endif

    CVI_MODEMNG_CONFIG_S stModemngCfg;
    stModemngCfg.pfnExitCB = CVI_ExitModeCallback;
    s32Ret = CVI_MODEMNG_Init(&stModemngCfg);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = CVI_ModuleDelayedStartThread();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    //open uvc mode
    //CVI_USB_SetMode(CVI_USB_MODE_UVC);

    s32Ret = CVI_NETCTRL_Init();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

#ifdef SERVICES_SPEECH_ON
    CVI_SPEECHMNG_PARAM_S SpeechParam;
    CVI_PARAM_GetSpeechParam(&SpeechParam);
    CVI_MAPI_ACAP_ATTR_S AiAttr = {0};
    CVI_PARAM_GetAiParam(&AiAttr);
    SpeechParam.AiNumPerFrm = AiAttr.u32PtNumPerFrm;
    s32Ret = CVI_SPEECHMNG_Init(&SpeechParam);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
    return 0;
}

static int32_t CVI_System_Exit()
{
    switch(s_ExitMode)
    {
        case CVI_MODEMNG_EXIT_MODE_POWEROFF:
        {
            CVI_LOGI("###### POWEROFF #####\n\n");
            reboot(RB_POWER_OFF);
            break;
        }
        case CVI_MODEMNG_EXIT_MODE_REBOOT:
        {
            CVI_LOGI("###### reboot #####\n\n");
            reboot(RB_AUTOBOOT);
            break;
        }
        default:
            CVI_LOGI("s_ExitMode error\n\n");
            reboot(RB_AUTOBOOT);
            break;
    }
    return 0;
}

static int32_t module_deinit(void)
{
    int32_t s32Ret = 0;
    return 0;
#ifdef CONFIG_SCREEN_ON
    s32Ret = CVI_HAL_SCREEN_Deinit(CVI_HAL_SCREEN_IDX_0);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

    s32Ret = CVI_MODEMNG_Deinit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#ifdef CONFIG_KEY_ON
    s32Ret = CVI_KEYMNG_DeInit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

#ifdef CONFIG_LED_ON
    s32Ret = CVI_LEDMNG_DeInit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
    s32Ret = CVI_MODEMNG_TEST_MAIN_Destroy();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = CVI_EVENTHUB_DeInit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = CVI_PARAM_Deinit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = CVI_System_Exit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = CVI_NETCTRL_DeInit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

#ifdef CONFIG_WATCHDOG_ON
    s32Ret = CVI_WATCHDOGMNG_DeInit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

#ifdef SERVICES_SPEECH_ON
    s32Ret = CVI_SPEECHMNG_DeInit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
	CVI_IPCM_Uninit();
	return 0;
}

#define use_ttys2 0

#if use_ttys2
#include <termios.h>
static int open_port(const char *port)
{
    int fd;
    struct termios options;

    // 打开串口设备
    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_port: Unable to open serial port");
        return -1;
    }

    // 配置串口参数
    tcgetattr(fd, &options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}
#endif

static void* media_thread(void* lparam)
{
	bool usb1_insert = 0;
	bool usb2_insert = 0;
	bool sd1_insert = 0;
	bool sd2_insert = 0;
	
	/*pthread_getschedparam( pthread_self(), &policy, &sched );
	sched.sched_priority = 0;
	pthread_setschedparam( pthread_self(), SCHED_RR, &sched );*/

	while ( 1 )
	{
		usleep(100*1000);
		if(!usb1_insert)
		{
			if(access("/dev/sda1",0) == 0)
			{
				system("mkdir /mnt/udisk");
				system("mount /dev/sda1 /mnt/udisk");
				usb1_insert = 1;
				printf("usb insert\n");
			}
		}
		else
		{
			if(access("/dev/sda1",0) != 0)
			{
				printf("usb out\n");
				if(system("umount /mnt/udisk") == 0/* && system("rm -r /mnt/udisk") == 0*/)
				{
					usb1_insert = 0;
				}
			}
		}
		if(!usb2_insert)
		{
			if(access("/dev/sdb1",0) == 0)
			{
				system("mkdir /mnt/usb");
				system("mount /dev/sdb1 /mnt/usb");
				usb2_insert = 1;
				printf("usb2 insert\n");
			}
		}
		else
		{
			if(access("/dev/sdb1",0) != 0)
			{
				printf("usb2 out\n");
				if(system("umount /mnt/usb") == 0/* && system("rm -r /mnt/usb") == 0*/)
				{
					usb2_insert = 0;
				}
			}
		}
		if(!sd1_insert)
		{
			if(access("/dev/mmcblk0p1",0) == 0)
			{
				//system("mkdir /mnt/sd");
				system("mount /dev/mmcblk0p1 /mnt/sd");
				sd1_insert = 1;
				printf("sd insert\n");
			}
		}
		else
		{
			if(access("/dev/mmcblk0p1",0) != 0)
			{
				printf("sd out\n");
				if(system("umount /mnt/sd") == 0/* && system("rm -r /mnt/sd") == 0*/)
				{
					sd1_insert = 0;
				}
			}
		}
		if(!sd2_insert)
		{
			if(access("/dev/mmcblk0p2",0) == 0)
			{
				//system("mkdir /mnt/sdmmc");
				system("mount /dev/mmcblk0p2 /mnt/sdmmc");
				sd2_insert = 1;
				printf("sd2 insert\n");
			}
		}
		else
		{
			if(access("/dev/mmcblk0p2",0) != 0)
			{
				printf("sd2 out\n");
				if(system("umount /mnt/sdmmc") == 0/* && system("rm -r /mnt/sdmmc") == 0*/)
				{
					sd2_insert = 0;
				}
			}
		}
	}
	
	return ((void *)0);
}

int32_t main(int32_t argc, char *argv[])
{
    int32_t s32Ret = 0;
#ifdef ENABLE_MTRACE
    setenv("MALLOC_TRACE","/mnt/system/mem.txt", 1);
    mtrace();
#endif
    signal(SIGPIPE, SIG_IGN);
    // signal_process();
    // mallopt(M_TRIM_THRESHOLD, 1024);
    /** init semaphore */
    sem_init(&s_PowerOffSem, 0, 0);

    module_init();
    
	pthread_t thread;
    pthread_create(&thread,NULL,media_thread,NULL);

    {
        //ui_common_SubscribeEvents();
/* option: to control fb options
 * - bit[0]: if true, double buffer
 * - bit[1]: if true, fb on vpss not vo
 * - bit[2:3]: 0:ARGB8888, 1:ARGB1555, 2:ARGB4444
 */
        int32_t  option = 0;
#if defined(FB_DUAL_BUFFER)
        option |= 0x1;
#endif
#if defined(FB_ARGB4444)
        option |= (0x2 << 2);
#elif defined(FB_ARGB1555)
       option |= (0x1 << 2);
#endif
        char cmd[64] = {0};
        snprintf(cmd, sizeof(cmd), "/mnt/system/ko/loadfbko.sh %d", option);
        printf("cvi_system %s\n",cmd);
        cvi_system(cmd);
    }

    /** insmod touchpad driver */
    s32Ret = cvi_insmod("/mnt/system/ko/3rd/gt9xx.ko",NULL);
    if(0 != s32Ret)
    {
        printf("insmod touchpad:failed, errno(%d)\n", errno);
        return -1;
    }

    if(access("/mnt/sd/App/CarApp",0) == 0)
    {
        //system("/mnt/sd/App/CarApp -qws");
        //system("/mnt/sd/App/CarApp");
    }
    else
    {
        //system("/mnt/system/bin/App/CarApp -qws");
        //system("/mnt/system/bin/App/CarApp");
    }

#if use_ttys2
    int fd;
    char buf[255];
    int n;

    // 打开串口设备
    fd = open_port("/dev/ttyS1");
    if (fd == -1) {

        printf("open err\n");
        exit(1);
    }
    printf("####### %s, %d\n", __func__, __LINE__);
#endif
    while((0 != sem_wait(&s_PowerOffSem)) && (errno == EINTR));
//     while(1) {
// #if use_ttys2
//         n = read(fd, buf, sizeof(buf));
//         printf("####### %s, %d,n:%d\n", __func__, __LINE__, n);
//         if (n > 0) {
//             printf("%.*s\n", n, buf);
//         }

//         // 发送串口数据
//         strcpy(buf, "Hello, world!\n");
//         n = write(fd, buf, strlen(buf));
//         if (n < 0) {
//             printf("write failed\n");
//         } else {
//             printf("write %d bytes\n", n);
//         }
//         sleep(1);
// #endif
//     }

    module_deinit();

    return s32Ret;
}
