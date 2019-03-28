FILE_DIR=$(shell /bin/pwd)
ROOT_DIR=$(FILE_DIR)/../..

ifndef TOP
TOP=$(ROOT_DIR)
export TOP
endif

include $(TOP)/make.include

LOCAL_INCLUDE += $(LOCAL_INCLUDE_DIR:%=-I$(ROOT_DIR)/%)  

LOCAL_INCLUDE += \
	-I./ 	\
	-I./include/ 	\
	-I$(ROOT_DIR)/include/	\
	-I$(ROOT_DIR)/include/jansson

CFLAGS += $(LOCAL_INCLUDE) 

DEFS = -DHAVE_CONFIG_H
CFLAGS += $(DEFS)  

TARGET		= $(LIB_DIR)/libjansson.a

LDFLAGS = -L$(LIB_DIR)/ -nostartfiles -L$(INSTALL_DIR)/lib -Ttarget.ld 

SOURCES =

SOURCES += 	dump.c \
	error.c \
	hashtable.c \
	hashtable_seed.c \
	load.c \
	memory.c \
	pack_unpack.c \
	strbuffer.c \
	strconv.c \
	utf.c \
	value.c

OBJS_NAME = $(basename $(SOURCES))

APP=N
include $(ROOT_DIR)/Makefile.in
