include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(OBJTREE)/bin
TARGET		:= $(TARGET_DIR)/mapi_osd_demo

############################################
# Dependencies
############################################
MAPI_DIR	:= $(SRCTREE)/framework/mapi
INCS		+= -I$(MAPI_DIR)/include
LIBS		+= -L$(MAPI_DIR)/lib -lcvi_mapi


HAL_DIR		:= $(SRCTREE)/cpsl/hal
INCS		+= -I$(HAL_DIR)/include
INCS		+= -I$(HAL_DIR)/screen/comm/include
INCS		+= -I$(HAL_DIR)/screen/mipidsi/include
INCS		+= -I$(HAL_DIR)/screen/hx8394_mipi/include

INCS		+= -I$(HAL_DIR)/gpio/include
INCS		+= -I$(HAL_DIR)/pwm/include
LIBS		+= -L$(HAL_DIR)/screen/lib -lcvi_hal_screen
LIBS		+= -L$(HAL_DIR)/pwm/lib -lcvi_hal_pwm
LIBS		+= -L$(HAL_DIR)/gpio/lib -lcvi_hal_gpio

UTILS_DIR	:= $(SRCTREE)/cpsl/common/sysutils
INCS		+= -I$(UTILS_DIR)/include
LIBS 		+= -L$(UTILS_DIR)/lib -lcvi_sysutils

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include
LIBS		+= -L$(LOG_DIR)/lib -lcvi_log

MMF_DIR		:= $(SRCTREE)/cpsl/mmf
LIBS		+= -L$(MMF_DIR)/lib -lmipi_tx
LIBS		+= -pthread

MMF_DIR		:= $(SRCTREE)/cpsl/mmf
INCS		+= -I$(MMF_DIR)/include
INCS		+= -I$(MMF_DIR)/sample/common
LIBS		+= -L$(MMF_DIR)/lib -L$(MMF_DIR)/lib/3rd -lsample -lcvi_bin -lcvi_bin_isp \
	-lvenc -lvdec -lcvi_jpeg -lcvi_vcodec \
	-Wl,--start-group -lsys -lvpu -lisp -lisp_algo -lawb -lae -laf -Wl,--end-group \
	-lsns_full -lmipi_tx -lcvi_audio -lcvi_vqe \
	-lcvi_VoiceEngine -lcvitracer -lini
# TODO: FIXME when .a are available
DYN_LIBS	+= -laec -lcvi_RES -ltinyalsa

MQ_DIR		:= $(SRCTREE)/cpsl/common/mq
INCS		+= -I$(MQ_DIR)/include
LIBS		+= -L$(MQ_DIR)/lib -lcvi_mq

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include
LIBS		+= -L$(OSAL_DIR)/lib -lcvi_osal

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include
LIBS		+= -L$(LOG_DIR)/lib -lcvi_log

DYN_LIBS	+= -lm -lstdc++ -pthread -ldl
############################################
# End of Dependencies
############################################
CFLAGS		+= -Wno-unused-parameter

SEXT		:= c
SDIR		:= $(CURDIR)
include $(SRCTREE)/build/common_exe.mk
