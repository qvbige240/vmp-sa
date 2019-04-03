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
	-I$(ROOT_DIR)/include/vmp/	\
	-I$(ROOT_DIR)/include/mw/ \
	-I$(ROOT_DIR)/include/common/  \
	-I$(ROOT_DIR)/include/utils/  \
	-I$(ROOT_DIR)/include/threadpool/ \
	-I$(ROOT_DIR)/include/tima/ \
	-I$(ROOT_DIR)/include/mw/support \
	-I$(ROOT_DIR)/include/mw/publish \
	-I$(ROOT_DIR)/include/http \
	
	#-I$(ROOT_DIR)/include/libevent/ 

CFLAGS += $(LOCAL_INCLUDE) 

TARGET		= $(LIB_DIR)/libmw.a

LDFLAGS = -L$(LIB_DIR)/ -nostartfiles -L$(INSTALL_DIR)/lib -Ttarget.ld 

SOURCES =

SOURCES +=  \
	context.c \
	tp.c \

OBJS_NAME = $(basename $(SOURCES))

APP=N
include $(ROOT_DIR)/Makefile.in
