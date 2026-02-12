CROSS_COMPILE = arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar

CUR_DIR     := $(shell pwd)
TOP_DIR     := $(CUR_DIR)/../../..
INSTALL_DIR ?= $(TOP_DIR)/install

SRC_DIR     := $(CUR_DIR)/src
INC_DIR     := $(CUR_DIR)/include
LIB_DIR     := $(CUR_DIR)/lib
OBJ_DIR     := $(CUR_DIR)/obj

SRCS         = $(wildcard $(SRC_DIR)/*.c)
OBJS         = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS         = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)
INCS         = -I$(INC_DIR)

TARGET       = $(LIB_DIR)/libcvi_minini.a
TARGET_SO    = $(LIB_DIR)/libcvi_minini.so
 
LIBS        += -lpthread -lm

ARFLAGS      = rcs
LDFLAGS_SO   = -shared
# CXXFLAGS    += -Wall -Wextra -Werror -pedantic-errors -D_GNU_SOURCE
CXXFLAGS    += -O3
CXXFLAGS    += -fPIC

.PHONY : clean all install
all : $(TARGET) $(TARGET_SO)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CXXFLAGS) $(INCS) -o $@ -c $<

$(TARGET) : $(OBJS)
	mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) -o $@ $^

$(TARGET_SO) : $(OBJS)
	mkdir -p $(LIB_DIR)
	$(CC) $(LDFLAGS_SO) -o $@ $^

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) $(TARGET_SO)

install:
	mkdir -p $(INSTALL_DIR) 
	mkdir -p $(INSTALL_DIR)/lib
	mkdir -p $(INSTALL_DIR)/include
	cp $(TARGET) $(INSTALL_DIR)/lib
	cp $(TARGET_SO) $(INSTALL_DIR)/lib
	cp $(CUR_DIR)/include/*.h $(INSTALL_DIR)/include

-include $(DEPS)
