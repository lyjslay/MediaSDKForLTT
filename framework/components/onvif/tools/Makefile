CROSS_COMPILE = arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar

CUR_DIR     := $(shell pwd)
SDIR 		:= $(CUR_DIR)/autogen
LIB_DIR     := $(SDIR)/lib
OBJ_DIR     := $(SDIR)/obj

SRCS = $(wildcard $(SDIR)/*.c)
OBJS = $(SRCS:$(SDIR)/%.c=$(OBJ_DIR)/%.o)
DEPS = $(SRCS:$(SDIR)/%.c=$(OBJ_DIR)/%.d)
TARGET       = $(LIB_DIR)/libonvif.a
TARGET_SO    = $(LIB_DIR)/libonvif.so

OPENSSL_DIR	:= ${SRCTREE}/cpsl/third_party/openssl
INCS		+= -I$(OPENSSL_DIR)/include
LIBS		+= -L$(OPENSSL_DIR)/lib -lssl -lcrypto
LIBS        += -lpthread -lm

ARFLAGS      = rcs
LDFLAGS_SO   = -shared
# CXXFLAGS    += -Wall -Wextra -Werror -pedantic-errors -D_GNU_SOURCE
# CXXFLAGS    += -O3
CXXFLAGS    += -fPIC -DWITH_OPENSSL

.PHONY : clean all install
all : $(TARGET) $(TARGET_SO)

$(OBJ_DIR)/%.o: $(SDIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CXXFLAGS) $(INCS) $(LIBS) -o $@ -c $<

$(TARGET) : $(OBJS)
	mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) -o $@ $^

$(TARGET_SO) : $(OBJS)
	mkdir -p $(LIB_DIR)
	$(CC) $(LDFLAGS_SO) -o $@ $^

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) $(TARGET_SO)

-include $(DEPS)
