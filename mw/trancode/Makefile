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
	-I$(ROOT_DIR)/include/vmp/ \
	-I$(ROOT_DIR)/include/mw/ \
	-I$(ROOT_DIR)/include/common/  \
	

CFLAGS += $(LOCAL_INCLUDE) 

TARGET		= $(LIB_DIR)/libtrancode.a

LDFLAGS = -L$(LIB_DIR)/ -nostartfiles -L$(INSTALL_DIR)/lib -Ttarget.ld 

SOURCES =

SOURCES +=  \
	audio_trancode.c \

OBJS_NAME = $(basename $(SOURCES))

APP=N
include $(ROOT_DIR)/Makefile.in
