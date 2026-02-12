include $(SRCTREE)/build/base.mk

SUBDIRS := thttpd cJSON
#.PHONY : clean all install
TOPTARGETS := all clean install

ifeq ($(CONFIG_FB_PIX_DEPTH_32), y)
	DEPTH=32
else
	DEPTH=16
endif

ifeq ($(CONFIG_TOUCHPAD), y)
	CVI_TOUCHPAD=y
else
	CVI_TOUCHPAD=n
endif

ifeq ($(CONFIG_RISCV), y)
	ARCH_RISCV=y
else
	ARCH_RISCV=n
endif

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS) CROSS_COMPILE_3RD=$(CROSS_COMPILE)

.PHONY: $(TOPTARGETS) $(SUBDIRS)

all:
ifeq ($(CONFIG_SCREEN), y)
	LIB_DIR=$(CVILIB_DIR) RISCV=$(ARCH_RISCV) FB_DEPTH=$(DEPTH) SDK_ROOT=$(SRCTREE) NPROC=$(NPROC) \
		TOUCHPAD=$(CVI_TOUCHPAD) CROSS_COMPILE_3RD=$(CROSS_COMPILE) $(CURDIR)/build_awtk.sh
endif
	mkdir -p $(CURDIR)/ffmpeg/build/$(CVILIB_DIR)
	BUILD_DIR=$(CURDIR)/ffmpeg/build/$(CVILIB_DIR) INSTALL_DIR=./install \
		CROSS_COMPILE_3RD=$(CROSS_COMPILE) $(CURDIR)/build_ffmpeg.sh
ifeq ($(CONFIG_ZBAR), y)
	mkdir -p $(CURDIR)/zbar-0.10/build/$(CVILIB_DIR)
	BUILD_DIR=$(CURDIR)/zbar-0.10 INSTALL_DIR=$(CURDIR)/zbar-0.10/build/$(CVILIB_DIR) RISCV=$(ARCH_RISCV) \
        CROSS_COMPILE_3RD=$(CROSS_COMPILE) $(CURDIR)/build_zbar.sh
endif

clean:
ifeq ($(CONFIG_SCREEN), y)
	rm -rf $(CURDIR)/awtkcode/awtk-linux-fb/build
	rm -rf $(CURDIR)/awtkcode/awtk-linux-fb/__pycache__
	rm -rf $(CURDIR)/awtkcode/awtk-linux-fb/.sconsign.dblite
endif
	rm -rf $(CURDIR)/ffmpeg/build
	rm -rf $(CURDIR)/ffmpeg/include
	rm -rf $(CURDIR)/ffmpeg/lib
	rm -rf $(CURDIR)/ffmpeg/bin
	rm -rf $(CURDIR)/ffmpeg/share
ifeq ($(CONFIG_ZBAR), y)
	rm -rf $(CURDIR)/zbar-0.10/build
	rm -rf $(CURDIR)/zbar-0.10/lib
	rm -rf $(CURDIR)/zbar-0.10/bin
endif
install:
	mkdir -p $(INSTALL_PATH)/lib
	cp -a $(CURDIR)/ffmpeg/lib/*.* $(INSTALL_PATH)/lib/
	# cp -a $(CURDIR)/flatbuffers/lib/* $(INSTALL_PATH)/lib/
	# cp -a $(CURDIR)/glog/lib/* $(INSTALL_PATH)/lib/
	# cp -a $(CURDIR)/live555/lib/* $(INSTALL_PATH)/lib/
	# cp -a $(CURDIR)/opencv/lib/* $(INSTALL_PATH)/lib/
	# cp -a $(CURDIR)/sqlite3/lib/* $(INSTALL_PATH)/lib/
	# cp -a $(CURDIR)/thttpd/lib/* $(INSTALL_PATH)/lib/
	# cp -a $(CURDIR)/zlib/lib/* $(INSTALL_PATH)/lib/
	# cp -a $(CURDIR)/minIni/lib/* $(INSTALL_PATH)/lib/
	cp -a $(CURDIR)/thttpd/lib/* $(INSTALL_PATH)/lib/
ifeq ($(CONFIG_SCREEN), y)
	cp -a $(CURDIR)/awtkcode/awtk-linux-fb/build/lib/libawtk.so $(INSTALL_PATH)/lib/
endif
ifeq ($(CONFIG_RISCV), y)
	cp -a $(CURDIR)/csi2d/$(CVILIB_DIR)/* $(INSTALL_PATH)/lib/
endif
