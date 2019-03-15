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
	-I$(ROOT_DIR)/include/mw/	\
	-I$(ROOT_DIR)/include/mw/support \

	#-I$(DEPEND_DIR)/include/vpk/ \
	

CFLAGS += $(LOCAL_INCLUDE) 

TARGET		= $(LIB_DIR)/libprotocol.a

LDFLAGS = -L$(LIB_DIR)/ -nostartfiles -L$(INSTALL_DIR)/lib -Ttarget.ld 

SOURCES =

SOURCES +=  \
	tima_h264_parse.c \
	tima_rtmp_packager_h264.c \
	tima_rtmp_publisher.c \
	tima_jt1078_parser.c \


OBJS_NAME = $(basename $(SOURCES))

APP=N
include $(ROOT_DIR)/Makefile.in
