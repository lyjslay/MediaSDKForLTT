# assumptions
# - SEXT is defined (c or cpp)
# - SDIR is defined
# - SRCS is optional
# - if INCS is defined, will be add to CFLAGS or CXXFLAGS
# - if LIBS is defined, will be add to LDFLAGS
SRCS ?= $(shell find $(SDIR) -type f -name "*.$(SEXT)")

ifeq ($(RUNTIME_ENV), pc)
CROSS_COMPILE	=
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR))_pc
else
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR))
endif

OBJS := $(addprefix $(ODIR)/,$(notdir $(SRCS:.$(SEXT)=.o)))
DEPS := $(addprefix $(ODIR)/,$(notdir $(SRCS:.$(SEXT)=.d)))

CFLAGS		+= $(INCS)
CXXFLAGS	+= $(INCS)
LDFLAGS		+= --sysroot=$(SRCTREE)/platform/ramdisk/sysroot/$(SYSROOT_PATH)
ifneq ($(CONFIG_TOOLCHAIN_GLIBC_ARM),y)
ELFFLAGS 	+= -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=$(LP_TYPE)
endif

.PHONY : all clean install
all : $(TARGET)

$(ODIR) :
	mkdir -p $@

$(ODIR)/%.o : $(SDIR)/%.c | $(ODIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ODIR)/%.o : $(SDIR)/%.cpp | $(ODIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET_DIR) :
	mkdir -p $@

$(TARGET) : $(OBJS) | $(TARGET_DIR)
ifeq ($(CONFIG_STATIC), y)
ifeq ($(RUNTIME_ENV), pc)
	$(CC) -o $@ $^ $(LIBS) $(DYN_LIBS)
else
	# use static lib by default
	$(CC) $(LDFLAGS) $(ELFFLAGS) -o  $@ $^ -Wl,-Bstatic $(LIBS) -Wl,-Bdynamic $(DYN_LIBS)
endif
else
	# use dynamic lib by default
ifeq ($(RUNTIME_ENV), pc)
	$(CC) -o $@ $^ $(LIBS) $(DYN_LIBS)
else
	$(CC) $(LDFLAGS)  $(ELFFLAGS) -o $@ $^ $(LIBS) $(DYN_LIBS)
endif
endif

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

$(INSTALL_PATH)/bin :
	mkdir -p $@

install: $(TARGET) | $(INSTALL_PATH)/bin
	cp $^ $(INSTALL_PATH)/bin

-include $(DEPS)
