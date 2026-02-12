#CROSS_COMPILE ?= aarch64-linux-gnu-
#CC = $(CROSS_COMPILE)gcc
#CXX = $(CROSS_COMPILE)g++
#AR = $(CROSS_COMPILE)ar

CUR_DIR     := $(shell pwd)
SRC_DIR     := $(CUR_DIR)
INC_DIR     := $(CUR_DIR)
LIB_DIR     := $(CUR_DIR)

SRCS         = $(wildcard $(SRC_DIR)/*.c)
OBJS         = $(SRCS:.c=.o)
DEPS         = $(SRCS:.c=.d)
INCS         = -I$(INC_DIR)

TARGET       = $(LIB_DIR)/libzf_log.a
TARGET_SO    = $(LIB_DIR)/libzf_log.so

ARFLAGS      = rcs
LDFLAGS_SO   = -shared
DEPFLAGS     = -MMD
CFLAGS       = -Wall -Wextra -Werror -pedantic-errors -std=c99 -D_GNU_SOURCE
CFLAGS      += -O3
CFLAGS      += $(DEPFLAGS)
CFLAGS      += -fPIC

.PHONY : clean all
all : $(TARGET) $(TARGET_SO)

$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) $(INCS) -o $@ -c $<

$(TARGET) : $(OBJS)
	$(AR) $(ARFLAGS) -o $@ $^

$(TARGET_SO) : $(OBJS)
	$(CC) $(LDFLAGS_SO) -o $@ $^

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) $(TARGET_SO)

-include $(DEPS)
