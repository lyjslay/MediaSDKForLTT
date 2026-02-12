# assumptions
# - SEXT is defined (c or cpp)
# - SDIR is defined
# - SRCS is optional
# - if INCS is defined, will be add to CFLAGS or CXXFLAGS
# - if LIBS is defined, will be add to LDFLAGS
ifeq ($(SRCS), )
SRCS ?= $(shell find $(SDIR) -type f -name "*.$(SEXT)")
endif
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR)/$(CVILIB_DIR))
OBJS := $(addprefix $(ODIR)/,$(notdir $(SRCS:.$(SEXT)=.o)))
DEPS := $(addprefix $(ODIR)/,$(notdir $(SRCS:.$(SEXT)=.d)))

CFLAGS		+= $(INCS) -fPIC
CXXFLAGS	+= $(INCS) -fPIC
ifneq ($(CONFIG_TOOLCHAIN_GLIBC_ARM),y)
CFLAGS		+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=$(LP_TYPE)
CXXFLAGS	+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=$(LP_TYPE)
else
CFLAGS		+= -mfpu=neon -ftree-vectorize -O3 -fopt-info-vec
endif
LDFLAGS_SO	+= --sysroot=$(SRCTREE)/platform/ramdisk/sysroot/$(SYSROOT_PATH)

.PHONY : all clean install
all : $(TARGET_A) $(TARGET_SO)

$(ODIR) :
	mkdir -p $@

$(ODIR)/%.o : $(SDIR)/%.c | $(ODIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ODIR)/%.o : $(SDIR)/%.cpp | $(ODIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

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
