# applications include
ifeq ($(CONFIG_SCREEN), y)
CFLAGS		+= -DCONFIG_SCREEN_ON

AWTK_DIR	:= $(SRCTREE)/cpsl/third_party/awtkcode
INCS		+= -I$(AWTK_DIR) -I$(AWTK_DIR)/awtk/src \
	-I$(AWTK_DIR)/awtk/src/ext_widgets -I$(AWTK_DIR)/awtk/src/base \
	-I$(AWTK_DIR)/awtk-linux-fb/awtk-port -DLOAD_ASSET_WITH_MMAP

TOUCH_DIR   := $(SRCTREE)/cpsl/hal/touchpad
INCS		+= -I$(TOUCH_DIR)/include
endif

UI_DIR		:= $(SRCTREE)/applications/dashcam/modules/ui/$(CFG_PDT_SUB)/$(UI_PACKET)
INCS		+= -I$(UI_DIR)/include

NETCTRL_DIRS := $(SRCTREE)/applications/dashcam/modules/netctrl
INCS         += -I$(NETCTRL_DIRS)/include

MODE_DIR 	:= $(SRCTREE)/applications/dashcam/modules/mode
INCS		+= -I$(MODE_DIR)/include

MEDIA_DIR   := $(SRCTREE)/applications/dashcam/modules/media
INCS		+= -I$(MEDIA_DIR)/include

USBCTRL_DIR := $(SRCTREE)/applications/dashcam/modules/usbctrl
INCS		+= -I$(USBCTRL_DIR)/include

ifeq ($(CONFIG_SERVICES_SPEECH), y)
SPEECHMNG_DIR	:= $(SRCTREE)/applications/common/speechmng
INCS        += -I$(SPEECHMNG_DIR)/include
endif

CVINET_DIR	:= $(SRCTREE)/applications/common/cvinet/
INCS        += -I$(CVINET_DIR)/include

SYSTEM_DIR  := $(SRCTREE)/applications/common/system
INCS		+= -I$(SYSTEM_DIR)/include

UTILS_DIR	:= $(SRCTREE)/applications/common/utils
INCS        += -I$(UTILS_DIR)/include

MD5_DIR1    := $(SRCTREE)/applications/common/utils/md5
INCS        += -I$(MD5_DIR1)/include

PARAM_DIR   := $(SRCTREE)/applications/dashcam/modules/param/core
INCS		+= -I$(PARAM_DIR)/include

RECMNG_DIR  := $(SRCTREE)/applications/common/recordmng
INCS		+= -I$(RECMNG_DIR)/include

ifeq ($(CONFIG_SERVICES_PHOTO), y)
PHOTOMNG_DIR:= $(SRCTREE)/applications/common/photomng
INCS		+= -I$(PHOTOMNG_DIR)/include
endif

ifeq ($(CONFIG_SERVICES_PLAYER), y)
PLAYMNG_DIR := $(SRCTREE)/applications/common/playbackmng
INCS		+= -I$(PLAYMNG_DIR)/include
endif

STGMNG_DIR  := $(SRCTREE)/applications/common/storagemng
INCS		+= -I$(STGMNG_DIR)/include

USB_DIR		:= $(SRCTREE)/applications/common/usb
INCS 		+= -I$(USB_DIR)/include

ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
VOLMNG_DIR	:= $(SRCTREE)/applications/common/volmng
INCS  		+= -I$(VOLMNG_DIR)/include
endif

FILEMNG_DIR	:= $(SRCTREE)/applications/common/filemng
INCS 		+= -I$(FILEMNG_DIR)/include

GPSMNG_DIR	:= $(SRCTREE)/applications/common/gpsmng
INCS 		+= -I$(GPSMNG_DIR)/include

ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
LVMNG_DIR	:= $(SRCTREE)/applications/common/liveviewmng
INCS 		+= -I$(LVMNG_DIR)/include
endif

TEMPER_DIR  := 	$(SRCTREE)/applications/common/tempermng
INCS		+= 	-I$(TEMPER_DIR)/include

DEVMNG_DIR  := $(SRCTREE)/applications/common/devmng
INCS        += -I$(DEVMNG_DIR)/include

CMDMNG_DIR  := $(SRCTREE)/applications/common/cmdmng
INCS        += -I$(CMDMNG_DIR)/include

POWER_DIR  	:= $(SRCTREE)/applications/common/powercontrol
INCS        += -I$(POWER_DIR)/include

TIMED_DIR	:= $(SRCTREE)/applications/common/utils/timedtask
INCS        += -I$(TIMED_DIR)/include

ifeq ($(CONFIG_SERVICES_ADAS), y)
ADASMNG_DIR	:= $(SRCTREE)/applications/common/adasmng
INCS        += -I$(ADASMNG_DIR)/include
endif

ifeq ($(CONFIG_VIDEO_MD), y)
VIDEOMD_DIR	:= $(SRCTREE)/applications/common/videomd/
INCS        += -I$(VIDEOMD_DIR)/include
endif

ifeq ($(CONFIG_ISP_IR_CUT), y)
ISPEXP_DIR	:= $(SRCTREE)/applications/common/ispircut/
INCS        += -I$(ISPEXP_DIR)/include
endif

# framework include
ifeq ($(CONFIG_SERVICES_RECORD), y)
CS_DIR		:= $(SRCTREE)/framework/services/recorder
INCS		+= -I$(CS_DIR)/include
endif

ACAPSER_DIR := $(SRCTREE)/framework/services/audio
INCS        += -I$(ACAPSER_DIR)/include

ifeq ($(CONFIG_SERVICES_SUBVIDEO), y)
VCAPSER_DIR   := $(SRCTREE)/framework/services/vcap_ser
INCS		+= -I$(VCAPSER_DIR)/include
endif

ifeq ($(CONFIG_SERVICES_RTSP), y)
RTSPSER_DIR := $(SRCTREE)/framework/services/rtsp_ser
INCS        += -I$(RTSPSER_DIR)/include
endif

ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
LIVEVIEW_DIR:= $(SRCTREE)/framework/services/liveview
INCS		+= -I$(LIVEVIEW_DIR)/include
endif

ifeq ($(CONFIG_SERVICES_PHOTO), y)
PHOTO_DIR	:= $(SRCTREE)/framework/services/photo
INCS		+= -I$(PHOTO_DIR)/include
endif

ifeq ($(CONFIG_SERVICES_PLAYER), y)
PS_DIR		:= $(SRCTREE)/framework/services/player
INCS		+= -I$(PS_DIR)/include
endif

STORAGE_DIR	:= $(SRCTREE)/framework/services/storage
INCS		+= -I$(STORAGE_DIR)/include

ifeq ($(CONFIG_SERVICES_ADAS), y)
ADAS_DIR	:= $(SRCTREE)/framework/services/adas
INCS        += -I$(ADAS_DIR)/include
endif

ifeq ($(CONFIG_SERVICES_SPEECH), y)
SPEECH_DIR		:= $(SRCTREE)/framework/services/speech
INCS		+= -I$(SPEECH_DIR)/include
endif

ifeq ($(CONFIG_SERVICES_QRCODE), y)
QRCODE_DIR		:= $(SRCTREE)/framework/services/qrcode
INCS		+= -I$(QRCODE_DIR)/include
endif

MAPI_DIR	:= $(SRCTREE)/framework/mapi
INCS		+= -I$(MAPI_DIR)/include

ifeq ($(CONFIG_COMPONENTS_PLAYER), y)
PLAYER_DIR	:= $(SRCTREE)/framework/components/player
INCS		+= -I$(PLAYER_DIR)/include
endif

ifeq ($(CONFIG_COMPONENTS_DEMUXER), y)
DEMUXER_DIR	:= $(SRCTREE)/framework/components/demuxer
INCS		+= -I$(DEMUXER_DIR)/include
endif

ifeq ($(CONFIG_COMPONENTS_FILE_RECOVER), y)
FILE_RECOVER_DIR:= $(SRCTREE)/framework/components/file_recover
INCS		+= -I$(FILE_RECOVER_DIR)/include
endif

PERF_DIR	:= $(SRCTREE)/framework/components/perf
INCS		+= -I$(PERF_DIR)/include

ifeq ($(CONFIG_COMPONENTS_RINGBUFFER), y)
RBUF_DIR    := $(SRCTREE)/framework/components/ringbuffer
INCS        += -L$(RBUF_DIR)/include
endif

ifeq ($(CONFIG_COMPONENTS_RECORDER), y)
RECR_DIR	:= $(SRCTREE)/framework/components/recorder
INCS		+= -I$(RECR_DIR)/include
endif

ifeq ($(CONFIG_COMPONENTS_MUXER), y)
MUXER_DIR	:= $(SRCTREE)/framework/components/muxer
INCS		+= -I$(MUXER_DIR)/include
endif

ifeq ($(CONFIG_COMPONENTS_THUMBNAIL_EXTRACTOR), y)
THMEXT_DIR  := $(SRCTREE)/framework/components/thumbnail_extractor
INCS        += -I$(THMEXT_DIR)/include
endif

# cpsl include
QUEUE_DIR	:= $(SRCTREE)/cpsl/common/queue
INCS        += -I$(QUEUE_DIR)/include

SYSUTILS_DIR:= $(SRCTREE)/cpsl/common/sysutils
INCS        += -I$(SYSUTILS_DIR)/include

STG_DIR		:= $(SRCTREE)/cpsl/common/storage
INCS		+= -I$(STG_DIR)/include

KEY_DIR		:= $(SRCTREE)/cpsl/hal/key
INCS		+= -I$(KEY_DIR)/include

LED_DIR     := $(SRCTREE)/cpsl/hal/led
INCS        += -I$(LED_DIR)/include

WATCHDOG_DIR  := $(SRCTREE)/cpsl/hal/watchdog
INCS          += -I$(WATCHDOG_DIR)/include

DTCF_DIR	:= $(SRCTREE)/cpsl/common/dtcf
INCS		+= -I$(DTCF_DIR)/include

TIMER_DIR	:= $(SRCTREE)/cpsl/common/timer
INCS		+= -I$(TIMER_DIR)/include

UPGRADE_DIR := $(SRCTREE)/cpsl/common/upgrade
INCS        += -I$(UPGRADE_DIR)/include

FFMPEG_DIR	:= $(SRCTREE)/cpsl/third_party/ffmpeg
INCS		+= -I$(FFMPEG_DIR)/include

ifeq ($(CONFIG_ZBAR), y)
ZBAR_DIR	:= $(SRCTREE)/cpsl/third_party/zbar-0.10
INCS		+= -I$(ZBAR_DIR)/include
endif

THTTPD_DIR	:= $(SRCTREE)/cpsl/third_party/thttpd
INCS		+= -I$(THTTPD_DIR)/include

CJSON_DIR	:= $(SRCTREE)/cpsl/third_party/cJSON
INCS		+= -I$(CJSON_DIR)/include

HAL_DIR		:= $(SRCTREE)/cpsl/hal
INCS		+= -I$(HAL_DIR)/screen/include
INCS		+= -I$(HAL_DIR)/screen/comm/include
INCS		+= -I$(HAL_DIR)/screen/mipidsi/include

INCS		+= -I$(HAL_DIR)/gpio/include
INCS		+= -I$(HAL_DIR)/pwm/include
INCS		+= -I$(HAL_DIR)/adc/include
INCS		+= -I$(HAL_DIR)/gsensor/include
INCS		+= -I$(HAL_DIR)/wifi/include
INCS 		+= -I$(HAL_DIR)/gps/include
INCS        += -I$(HAL_DIR)/include

HAL_GPIO_DIR	:= $(HAL_DIR)/gpio
INCS		+= -I$(HAL_GPIO_DIR)/comm/include

FLASH_DIR	:= $(SRCTREE)/cpsl/hal/flash
INCS		+= -I$(FLASH_DIR)/include

MMF_DIR		:= $(SRCTREE)/cpsl/mmf
INCS		+= -I$(MMF_DIR)/include

FHSM_DIR	:= $(SRCTREE)/cpsl/common/hfsm
INCS		+= -I$(FHSM_DIR)/include

EHUB_DIR	:= $(SRCTREE)/cpsl/common/eventhub
INCS		+= -I$(EHUB_DIR)/include

MQ_DIR		:= $(SRCTREE)/cpsl/common/mq
INCS		+= -I$(MQ_DIR)/include

SIGSLOT_DIR	:= $(SRCTREE)/cpsl/common/signal_slot
INCS		+= -I$(SIGSLOT_DIR)/include

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include

ifeq ($(CONFIG_TPU), y)
AI_DIR		:= $(SRCTREE)/cpsl/tpu/cvitek_ai_sdk
INCS		+= -I$(AI_DIR)/include/cvi_tdl
INCS		+= -I$(AI_DIR)/include/cvi_tdl_app
INCS		+= -I$(AI_DIR)/sample/3rd/opencv/include
endif

INCS		+= -I$(CURDIR)/include
