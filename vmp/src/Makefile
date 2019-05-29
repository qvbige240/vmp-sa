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
	-I$(ROOT_DIR)/include/	\
	-I$(ROOT_DIR)/include/vmp \

CFLAGS += $(LOCAL_INCLUDE) 

DEFS = -DHAVE_CONFIG_H
CFLAGS += $(DEFS)  

TARGET		= $(LIB_DIR)/libvmp.a

LDFLAGS = -L$(LIB_DIR)/ -nostartfiles -L$(INSTALL_DIR)/lib -Ttarget.ld 

SOURCES =

SOURCES += 	vpk_util.c \
			vpk_libc.c \
			vpk_ifilesys.c \
			vpk_sock.c \
			vmp_node.c \
			vmp_vector.c \

OBJS_NAME = $(basename $(SOURCES))

APP=N
include $(ROOT_DIR)/Makefile.in
