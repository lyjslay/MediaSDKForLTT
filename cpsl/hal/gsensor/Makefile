include $(SRCTREE)/build/base.mk

gsensor-y += common/src
gsensor-$(CONFIG_GSENSOR_DA280)              += da280
gsensor-$(CONFIG_GSENSOR_DA380)              += da380
gsensor-$(CONFIG_GSENSOR_SC7A20E)              += sc7a20e
SEXT := c
SDIR := $(CURDIR)
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR))
SRCS := $(foreach dir,$(gsensor-y),$(wildcard $(dir)/*.$(SEXT)))
OBJS := $(addprefix $(ODIR)/,$(SRCS:.c=.o))
DEPS := $(addprefix $(ODIR)/,$(SRCS:.c=.d))
# $(info SDIR = $(SDIR))
# $(info ODIR = $(ODIR))
# $(info SRCS = $(SRCS))
# $(info OBJS = $(OBJS))

TARGET_DIR	:= $(CURDIR)/$(CVILIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_gsensor.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_gsensor.so

INCS		+= -I$(CURDIR)/include

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
