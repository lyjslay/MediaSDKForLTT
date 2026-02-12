include $(SRCTREE)/build/base.mk
CUR_DIR     := $(shell pwd)
AI_PATH     := $(CUR_DIR)/cvitek_ai_sdk
TPU_PATH    := $(CUR_DIR)/cvitek_tpu_sdk

.PHONY : clean all install
all:

clean:

install:
ifneq ($(CONFIG_STATIC), y)
ifeq ($(CONFIG_SERVICES_SPEECH), y)
	cp -a $(AI_PATH)/lib/libcvi_tdl.so $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/lib/libcvi_tdl_app.so $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_core.so.3.2 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_imgcodecs.so.3.2 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_imgproc.so.3.2 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_core.so.3.2.0 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_imgcodecs.so.3.2.0 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_imgproc.so.3.2.0 $(INSTALL_PATH)/lib/
endif

ifeq ($(CONFIG_TPU), y)
	cp -a $(TPU_PATH)/lib/libcnpy.so $(INSTALL_PATH)/lib/
	cp -a $(TPU_PATH)/lib/libcvikernel.so $(INSTALL_PATH)/lib/
	cp -a $(TPU_PATH)/lib/libcvimath.so $(INSTALL_PATH)/lib/
	cp -a $(TPU_PATH)/lib/libcviruntime.so $(INSTALL_PATH)/lib/
	cp -a $(TPU_PATH)/lib/libz.so.* $(INSTALL_PATH)/lib/
endif

ifeq ($(CONFIG_SERVICES_ADAS), y)
	cp -a $(AI_PATH)/lib/libcvi_tdl.so $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/lib/libcvi_tdl_app.so $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_core.so.3.2 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_imgcodecs.so.3.2 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_imgproc.so.3.2 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_core.so.3.2.0 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_imgcodecs.so.3.2.0 $(INSTALL_PATH)/lib/
	cp -a $(AI_PATH)/sample/3rd/opencv/lib/libopencv_imgproc.so.3.2.0 $(INSTALL_PATH)/lib/
endif

endif