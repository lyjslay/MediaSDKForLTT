include $(SRCTREE)/applications/build/base.mk

all:
clean:
install:
ifeq ($(CONFIG_BOARD_DEBUG),y)
	chmod 777 *.sh 
	mkdir -p $(INSTALL_PATH)/bin/test_scripts
	cp *.sh $(INSTALL_PATH)/bin/test_scripts
endif