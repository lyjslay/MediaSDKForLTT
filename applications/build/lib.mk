MODE_DIR 	:= $(SRCTREE)/applications/dashcam/modules/mode
LIBS        += -L$(MODE_DIR)/$(CVILIB_DIR) -lcvi_mode

UI_DIR		:= $(SRCTREE)/applications/dashcam/modules/ui/$(CFG_PDT_SUB)/$(UI_PACKET)
LIBS		+= -L$(UI_DIR)/$(CVILIB_DIR) -lcar_ui

ifeq ($(CONFIG_SCREEN), y)
CFLAGS		+= -DCONFIG_SCREEN_ON

AWTK_DIR	:= $(SRCTREE)/cpsl/third_party/awtkcode
LIBS		+= -L$(AWTK_DIR)/awtk-linux-fb/build/lib -lawtk

ifeq ($(CONFIG_TOUCHPAD), y)
TOUCH_DIR   := $(SRCTREE)/cpsl/hal/touchpad
LIBS		+= -L$(TOUCH_DIR)/$(CVILIB_DIR) -lcvi_hal_touchpad
endif
endif

MEDIA_DIR   := $(SRCTREE)/applications/dashcam/modules/media
LIBS        += -L$(MEDIA_DIR)/$(CVILIB_DIR) -lcvi_media

USBCTRL_DIR := $(SRCTREE)/applications/dashcam/modules/usbctrl
LIBS        += -L$(USBCTRL_DIR)/$(CVILIB_DIR) -lcvi_usbctrl

SYSTEM_DIR  := $(SRCTREE)/applications/common/system
LIBS        += -L$(SYSTEM_DIR)/$(CVILIB_DIR) -lcvi_system

PARAM_DIR   := $(SRCTREE)/applications/dashcam/modules/param/core
LIBS        += -L$(PARAM_DIR)/$(CVILIB_DIR) -lcvi_param

NETCTRL_DIR := $(SRCTREE)/applications/dashcam/modules/netctrl
LIBS        += -L$(NETCTRL_DIR)/$(CVILIB_DIR) -lcvi_netctrl

RECMNG_DIR  := $(SRCTREE)/applications/common/recordmng
LIBS        += -L$(RECMNG_DIR)/$(CVILIB_DIR) -lcvi_recordmng

ifeq ($(CONFIG_SERVICES_PHOTO), y)
PHOTOMNG_DIR:= $(SRCTREE)/applications/common/photomng
LIBS        += -L$(PHOTOMNG_DIR)/$(CVILIB_DIR) -lcvi_photomng
endif

ifeq ($(CONFIG_SERVICES_PLAYER), y)
PLAYMNG_DIR := $(SRCTREE)/applications/common/playbackmng
LIBS        += -L$(PLAYMNG_DIR)/$(CVILIB_DIR) -lcvi_playbackmng
endif

STGMNG_DIR  := $(SRCTREE)/applications/common/storagemng
LIBS        += -L$(STGMNG_DIR)/$(CVILIB_DIR) -lcvi_stgmng

USB_DIR		:= $(SRCTREE)/applications/common/usb
LIBS        += -L$(USB_DIR)/$(CVILIB_DIR) -lcvi_usb

ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
VOLMNG_DIR	:= $(SRCTREE)/applications/common/volmng
LIBS        += -L$(VOLMNG_DIR)/$(CVILIB_DIR) -lcvi_volmng
endif

FILEMNG_DIR	:= $(SRCTREE)/applications/common/filemng
LIBS        += -L$(FILEMNG_DIR)/$(CVILIB_DIR) -lcvi_filemng

ifeq ($(CONFIG_GPS), y)
GPSMNG_DIR	:= $(SRCTREE)/applications/common/gpsmng
LIBS        += -L$(GPSMNG_DIR)/$(CVILIB_DIR) -lcvi_gpsmng
endif

ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
LVMNG_DIR	:= $(SRCTREE)/applications/common/liveviewmng
LIBS        += -L$(LVMNG_DIR)/$(CVILIB_DIR) -lcvi_liveviewmng
endif

TEMPERMNG_DIR  	:= 	$(SRCTREE)/applications/common/tempermng
LIBS        	+= 	-L$(TEMPERMNG_DIR)/$(CVILIB_DIR) -lcvi_tempermng

DEVMNG_DIR  := $(SRCTREE)/applications/common/devmng
LIBS        += -L$(DEVMNG_DIR)/$(CVILIB_DIR) -lcvi_devmng

CMDMNG_DIR  := $(SRCTREE)/applications/common/cmdmng
LIBS        += -L$(CMDMNG_DIR)/$(CVILIB_DIR) -lcvi_cmdmng

POWER_DIR  	:= $(SRCTREE)/applications/common/powercontrol
LIBS        += -L$(POWER_DIR)/$(CVILIB_DIR) -lcvi_powercontrol

MD5_DIR	:= $(SRCTREE)/applications/common/utils/md5
LIBS        += -L$(MD5_DIR)/$(CVILIB_DIR) -lcvi_md5

TIMED_DIR	:= $(SRCTREE)/applications/common/utils/timedtask
LIBS        += -L$(TIMED_DIR)/$(CVILIB_DIR) -lcvi_timedtask

NET_DIR     := $(SRCTREE)/applications/common/cvinet
LIBS        += -L$(NET_DIR)/$(CVILIB_DIR) -lcvi_net

ifeq ($(CONFIG_SERVICES_SPEECH), y)
SPEECHMNG_DIR := $(SRCTREE)/applications/common/speechmng
LIBS        += -L$(SPEECHMNG_DIR)/$(CVILIB_DIR) -lcvi_speechmng
endif

ifeq ($(CONFIG_SERVICES_ADAS), y)
ADASMNG_DIR	:= $(SRCTREE)/applications/common/adasmng
LIBS        += -L$(ADASMNG_DIR)/$(CVILIB_DIR) -lcvi_adasmng
endif

ifeq ($(CONFIG_VIDEO_MD), y)
VIDEOMD_DIR	:= $(SRCTREE)/applications/common/videomd
LIBS        += -L$(VIDEOMD_DIR)/$(CVILIB_DIR) -lcvi_videomd
endif

ifeq ($(CONFIG_ISP_IR_CUT), y)
ISPEXP_DIR	:= $(SRCTREE)/applications/common/ispircut/
LIBS        += -L$(ISPEXP_DIR)/$(CVILIB_DIR) -lcvi_ispircut
endif

ifeq ($(CONFIG_SERVICES_RECORD), y)
CS_DIR		:= $(SRCTREE)/framework/services/recorder
LIBS		+= -L$(CS_DIR)/$(CVILIB_DIR) -lcvi_record_service
endif

ifeq ($(CONFIG_SERVICES_PHOTO), y)
PHOTO_DIR	:= $(SRCTREE)/framework/services/photo
LIBS		+= -L$(PHOTO_DIR)/$(CVILIB_DIR) -lcvi_photo_service
endif

ifeq ($(CONFIG_SERVICES_AUDIO), y)
ACAPSER_DIR := $(SRCTREE)/framework/services/audio
LIBS		+= -L$(ACAPSER_DIR)/$(CVILIB_DIR) -lcvi_audio_service
endif

ifeq ($(CONFIG_SERVICES_SUBVIDEO), y)
VCAPSER_DIR := $(SRCTREE)/framework/services/vcap_ser
LIBS		+= -L$(VCAPSER_DIR)/$(CVILIB_DIR) -lvcap
endif

ifeq ($(CONFIG_SERVICES_RTSP), y)
RTSPSER_DIR := $(SRCTREE)/framework/services/rtsp_ser
LIBS		+= -L$(RTSPSER_DIR)/$(CVILIB_DIR) -lcvi_rtsp_service
endif

ifeq ($(CONFIG_SERVICES_LIVEVIEW), y)
LIVEVIEW_DIR:= $(SRCTREE)/framework/services/liveview
LIBS		+= -L$(LIVEVIEW_DIR)/$(CVILIB_DIR) -lcvi_liveview
endif

ifeq ($(CONFIG_SERVICES_PLAYER), y)
PS_DIR		:= $(SRCTREE)/framework/services/player
LIBS		+= -L$(PS_DIR)/$(CVILIB_DIR) -lcvi_player_service
endif

ifeq ($(CONFIG_SERVICES_STORAGE), y)
STORAGE_DIR	:= $(SRCTREE)/framework/services/storage
LIBS		+= -L$(STORAGE_DIR)/$(CVILIB_DIR) -lcvi_storage
endif

ifeq ($(CONFIG_SERVICES_SPEECH), y)
SPEECH_DIR := $(SRCTREE)/framework/services/speech
LIBS		+= -L$(SPEECH_DIR)/$(CVILIB_DIR) -lcvi_speech
endif

ifeq ($(CONFIG_SERVICES_ADAS), y)
ADAS_DIR := $(SRCTREE)/framework/services/adas
LIBS		+= -L$(ADAS_DIR)/$(CVILIB_DIR) -lcvi_adas_service
endif

ifeq ($(CONFIG_SERVICES_QRCODE), y)
QRCODE_DIR	:= $(SRCTREE)/framework/services/qrcode
LIBS		+= -L$(QRCODE_DIR)/$(CVILIB_DIR) -lcvi_qrcode_service
endif

MAPI_DIR	:= $(SRCTREE)/framework/mapi
LIBS		+= -L$(MAPI_DIR)/$(CVILIB_DIR) -lcvi_mapi

ifeq ($(CONFIG_COMPONENTS_PLAYER), y)
PLAYER_DIR	:= $(SRCTREE)/framework/components/player
LIBS		+= -L$(PLAYER_DIR)/$(CVILIB_DIR) -lcvi_player
endif

ifeq ($(CONFIG_COMPONENTS_DEMUXER), y)
DEMUXER_DIR	:= $(SRCTREE)/framework/components/demuxer
LIBS		+= -L$(DEMUXER_DIR)/$(CVILIB_DIR) -lcvi_demuxer
endif

ifeq ($(CONFIG_COMPONENTS_FILE_RECOVER), y)
FILE_RECOVER_DIR:= $(SRCTREE)/framework/components/file_recover
LIBS		+= -L$(FILE_RECOVER_DIR)/$(CVILIB_DIR) -lcvi_file_recover
endif

ifeq ($(CONFIG_COMPONENTS_PERF), y)
PERF_DIR	:= $(SRCTREE)/framework/components/perf
LIBS		+= -L$(PERF_DIR)/$(CVILIB_DIR) -lcvi_perf
endif

ifeq ($(CONFIG_COMPONENTS_RECORDER), y)
RECR_DIR	:= $(SRCTREE)/framework/components/recorder
LIBS		+= -L$(RECR_DIR)/$(CVILIB_DIR) -lcvi_recorder
endif

ifeq ($(CONFIG_COMPONENTS_MUXER), y)
MUXER_DIR	:= $(SRCTREE)/framework/components/muxer
LIBS		+= -L$(MUXER_DIR)/$(CVILIB_DIR) -lcvi_muxer
endif

ifeq ($(CONFIG_COMPONENTS_RINGBUFFER), y)
RBUF_DIR    := $(SRCTREE)/framework/components/ringbuffer
LIBS        += -L$(RBUF_DIR)/$(CVILIB_DIR) -lcvi_ringbuffer
endif

ifeq ($(CONFIG_COMPONENTS_THUMBNAIL_EXTRACTOR), y)
THMEXT_DIR  := $(SRCTREE)/framework/components/thumbnail_extractor
LIBS		+= -L$(THMEXT_DIR)/$(CVILIB_DIR) -lcvi_thumbnail_extractor
endif

MEMORY_ALLOCATOR_DIR := $(SRCTREE)/framework/components/mem_alloc
LIBS		+= -L$(MEMORY_ALLOCATOR_DIR)/$(CVILIB_DIR) -lcvi_mem_alloc

THPPPD_DIR  := $(SRCTREE)/cpsl/third_party/thttpd
LIBS        += -L$(THPPPD_DIR)/lib -lthttpd

ifeq ($(CONFIG_TPU), y)
AI_DIR   := $(SRCTREE)/cpsl/tpu/cvitek_ai_sdk
TPU_DIR  := $(SRCTREE)/cpsl/tpu/cvitek_tpu_sdk
OPENCV   += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs

# ifeq ($(CONFIG_TOOLCHAIN_GLIBC_ARM), y)
# OPENCV   += -ltegra_hal
# endif

ifeq ($(CONFIG_STATIC), y)
LIBS        += -L$(AI_DIR)/lib -lcvi_tdl -lcvi_tdl_app
LIBS      	+= -L$(AI_DIR)/sample/3rd/opencv/lib $(OPENCV)
LIBS        += -Wl,--start-group -L$(TPU_DIR)/lib -lcnpy  -lcvikernel-static  -lcvimath-static   -lcviruntime-static  -lz\
				-L$(MMF_DIR)/$(CVILIB_DIR) -lcvi_ive \
               -Wl,--end-group
else
LIBS        += -L$(AI_DIR)/lib -lcvi_tdl_app -lcvi_tdl
LIBS      	+= -L$(AI_DIR)/sample/3rd/opencv/lib $(OPENCV)
LIBS        += -Wl,--start-group -L$(TPU_DIR)/lib -lcnpy  -lcvikernel  -lcvimath  -lcviruntime  -lz\
				-L$(MMF_DIR)/$(CVILIB_DIR) -lcvi_ive \
               -Wl,--end-group

endif

endif

SYSUTILS_DIR:= $(SRCTREE)/cpsl/common/sysutils
LIBS        += -L$(SYSUTILS_DIR)/$(CVILIB_DIR) -lcvi_sysutils

QUEUE_DIR	:= $(SRCTREE)/cpsl/common/queue
LIBS        += -L$(QUEUE_DIR)/$(CVILIB_DIR) -lcvi_queue

STG_DIR		:= $(SRCTREE)/cpsl/common/storage
LIBS		+= -L$(STG_DIR)/$(CVILIB_DIR) -lcvi_stg

DTCF_DIR	:= $(SRCTREE)/cpsl/common/dtcf
LIBS		+= -L$(DTCF_DIR)/$(CVILIB_DIR) -ldtcf

TIMER_DIR	:= $(SRCTREE)/cpsl/common/timer
LIBS		+= -L$(TIMER_DIR)/$(CVILIB_DIR) -lcvi_timer

UPGRADE_DIR := $(SRCTREE)/cpsl/common/upgrade
LIBS		+= -L$(UPGRADE_DIR)/$(CVILIB_DIR) -lcvi_upgrade

ifeq ($(CONFIG_ZBAR), y)
ZBAR_DIR    := $(SRCTREE)/cpsl/third_party/zbar-0.10
LIBS        += -L$(ZBAR_DIR)/lib -lzbar
endif

FFMPEG_DIR	:= $(SRCTREE)/cpsl/third_party/ffmpeg
LIBS		+= -L$(FFMPEG_DIR)/lib \
	-lavformat -lavcodec -lavutil -lswresample

ifeq ($(CONFIG_RISCV), y)
CSI2D_DIR	:= $(SRCTREE)/cpsl/third_party/csi2d
LIBS		+= -L$(CSI2D_DIR)/$(CVILIB_DIR) -lcsi2d
endif

HAL_DIR		:= $(SRCTREE)/cpsl/hal
ifeq ($(CONFIG_SCREEN), y)
LIBS		+= -L$(HAL_DIR)/screen/$(CVILIB_DIR) -lcvi_hal_screen
endif

ifeq ($(CONFIG_KEY), y)
LIBS		+= -L$(HAL_DIR)/key/$(CVILIB_DIR) -lcvi_hal_key
endif

ifeq ($(CONFIG_LED), y)
LIBS		+= -L$(HAL_DIR)/led/$(CVILIB_DIR) -lcvi_hal_led
endif

ifeq ($(CONFIG_PWM), y)
LIBS		+= -L$(HAL_DIR)/pwm/$(CVILIB_DIR) -lcvi_hal_pwm
endif

ifeq ($(CONFIG_ADC), y)
LIBS		+= -L$(HAL_DIR)/adc/$(CVILIB_DIR) -lcvi_hal_adc
endif

ifeq ($(CONFIG_GPIO), y)
LIBS		+= -L$(HAL_DIR)/gpio/$(CVILIB_DIR) -lcvi_hal_gpio
endif

ifeq ($(CONFIG_GPS), y)
LIBS		+= -L$(HAL_DIR)/gps/$(CVILIB_DIR) -lcvi_hal_gps
LIBS		+= -L$(HAL_DIR)/uart/$(CVILIB_DIR) -lcvi_hal_uart
endif

ifeq ($(CONFIG_GSENSOR), y)
LIBS		+= -L$(HAL_DIR)/gsensor/$(CVILIB_DIR) -lcvi_hal_gsensor
endif

ifeq ($(CONFIG_WIFI), y)
LIBS        += -L$(HAL_DIR)/wifi/$(CVILIB_DIR) -lcvi_hal_wifi
endif

ifeq ($(CONFIG_WATCHDOG), y)
LIBS        += -L$(HAL_DIR)/watchdog/$(CVILIB_DIR) -lcvi_hal_watchdog
endif

LIBS        += -L$(HAL_DIR)/flash/$(CVILIB_DIR) -lcvi_hal_flash

# MMF_DIR		:= $(SRCTREE)/cpsl/mmf
# LIBS		+= -L$(MMF_DIR)/$(CVILIB_DIR) -lmipi_tx

ifeq ($(CONFIG_ENABLE_ISP_PQ_TOOL), y)
LIBS		+= -L$(MMF_DIR)/lib -lcvi_ispd2
LIBS		+= -L$(MMF_DIR)/lib/3rd -lcvi_json-c
endif

SENSOR_DIR	:= $(MMF_DIR)/component/isp/sensor/
LIBS		+= -L$(SENSOR_DIR)/lib_linux -lsensor_cfg

LIBS		+= -L$(MMF_DIR)/lib -L$(MMF_DIR)/lib/3rd \
	-lvenc -lvdec \
	-Wl,--start-group -lsys -lmsg -lcvilink -lcvi_bin -lcvi_bin_isp -lvi -lvpss -lvo -lrgn -lgdc -lisp -lawb -lae -laf -Wl,--end-group \
 	-lcvi_audio -lcvi_vqe -lipcm \
	-lcvi_VoiceEngine -lini

ifeq ($(CONFIG_SBC), y)
LIBS		+= -L$(MMF_DIR)/lib/3rd -lsbc
endif

ifeq ($(CONFIG_VIDEO_MD), y)
LIBS		+= -L$(MMF_DIR)/$(CVILIB_DIR) -lcvi_ive
endif

LIBS		+= -lcvi_RES1 -ltinyalsa -lcvi_ssp

FHSM_DIR	:= $(SRCTREE)/cpsl/common/hfsm
LIBS		+= -L$(FHSM_DIR)/$(CVILIB_DIR) -lcvi_hfsm

EHUB_DIR	:= $(SRCTREE)/cpsl/common/eventhub
LIBS		+= -L$(EHUB_DIR)/$(CVILIB_DIR) -lcvi_eventhub

MQ_DIR		:= $(SRCTREE)/cpsl/common/mq
LIBS		+= -L$(MQ_DIR)/$(CVILIB_DIR) -lcvi_mq

SIGSLOT_DIR	:= $(SRCTREE)/cpsl/common/signal_slot
LIBS		+= -L$(SIGSLOT_DIR)/$(CVILIB_DIR) -lcvi_signal_slot

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
LIBS		+= -L$(OSAL_DIR)/$(CVILIB_DIR) -lcvi_osal

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
LIBS		+= -L$(LOG_DIR)/$(CVILIB_DIR) -lcvi_log

THPPPD_DIR  := $(SRCTREE)/cpsl/third_party/thttpd
LIBS        += -L$(THPPPD_DIR)/lib -lthttpd

CJSON_DIR  	:= $(SRCTREE)/cpsl/third_party/cJSON
LIBS        += -L$(CJSON_DIR)/$(CVILIB_DIR) -lcJSON

DYN_LIBS	+= -lm -lstdc++ -lstdc++fs -pthread -ldl
