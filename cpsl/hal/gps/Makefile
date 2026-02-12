include $(SRCTREE)/build/base.mk

gps-y += src/common
gps-y += src/uart

SEXT := c
SDIR := $(CURDIR)
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR))
SRCS := $(foreach dir,$(gps-y),$(wildcard $(dir)/*.$(SEXT)))
OBJS := $(addprefix $(ODIR)/,$(SRCS:.c=.o))
DEPS := $(addprefix $(ODIR)/,$(SRCS:.c=.d))

LOG_DIR     := $(SRCTREE)/cpsl/common/log
INCS        += -I$(LOG_DIR)/include

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_gps.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_gps.so

INCS		+= -I$(CURDIR)/include
INCS		+= -I$(CURDIR)/src/common/
INCS		+= -I$(SRCTREE)/cpsl/hal/uart/include
# CFLAGS		+= -Wno-unused-result
CFLAGS      += $(INCS)
CFLAGS      += -fPIC
.PHONY : clean all install
all : $(TARGET_A) $(TARGET_SO)

$(ODIR)/%.o : $(SDIR)/%.$(SEXT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET_DIR) :
	mkdir -p $@

$(TARGET_A) : $(OBJS) | $(TARGET_DIR)
	$(AR) $(ARFLAGS) $@ $^

$(TARGET_SO) : $(OBJS) | $(TARGET_DIR)
	$(CC) $(LDFLAGS_SO) -o $@ $^

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)

$(INSTALL_PATH)/lib :
	mkdir -p $@

install: $(TARGET_A) $(TARGET_SO) | $(INSTALL_PATH)/lib
	cp $^ $(INSTALL_PATH)/lib

-include $(DEPS)

