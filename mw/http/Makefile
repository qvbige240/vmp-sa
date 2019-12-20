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
	-I./server/ 	\
	-I$(ROOT_DIR)/include/	\
	-I$(ROOT_DIR)/include/vmp/ \
	-I$(ROOT_DIR)/include/utils/  \
	-I$(ROOT_DIR)/include/http/ \
	-I$(ROOT_DIR)/include/mw/	\
	-I$(ROOT_DIR)/include/mw/support \
	-I$(ROOT_DIR)/include/cache/ \

	#-I$(DEPEND_DIR)/include/vpk/ \
	

CFLAGS += $(LOCAL_INCLUDE) 

TARGET		= $(LIB_DIR)/libhttp.a

LDFLAGS = -L$(LIB_DIR)/ -nostartfiles -L$(INSTALL_DIR)/lib -Ttarget.ld 

SOURCES =

SOURCES +=  \
	tima_http_client.c \
	tima_http.c \
	http_token.c \
	tima_get_property.c \
	server/http_demo.c \
	server/http_handler.c \
	server/http_server.c

OBJS_NAME = $(basename $(SOURCES))

APP=N
include $(ROOT_DIR)/Makefile.in
