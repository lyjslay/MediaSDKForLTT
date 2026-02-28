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

TARGET       = $(LIB_DIR)/libonvif_server.a
TARGET_SO    = $(LIB_DIR)/libonvif_server.so

############################################
# Dependencies
############################################
ONVIF_DIR	:= $(CUR_DIR)/tools/autogen
INCS		+= 	-I$(CUR_DIR)/tools/autogen
LIBS		+= 	-L$(CUR_DIR)/tools/autogen/lib -lonvif

OPENSSL_DIR	:= ${SRCTREE}/cpsl/third_party/openssl
INCS		+= -I$(OPENSSL_DIR)/include
LIBS		+= -L$(OPENSSL_DIR)/lib -lssl -lcrypto

LIBS        += -lpthread -lm
############################################
# End of Dependencies
############################################

ARFLAGS      = rcs
LDFLAGS_SO   = -shared
# CXXFLAGS    += -Wall -Wextra -Werror -pedantic-errors -D_GNU_SOURCE
CXXFLAGS    += -O3
CXXFLAGS    += -fPIC -DWITH_OPENSSL

.PHONY : clean all install
all : $(TARGET) $(TARGET_SO)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
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

install:

-include $(DEPS)
