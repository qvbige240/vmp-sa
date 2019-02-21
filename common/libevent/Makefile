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
	-I$(ROOT_DIR)/include/libevent \
	-I$(ROOT_DIR)/include/libevent/event2 \
	-I$(ROOT_DIR)/include/libevent/sys

CFLAGS += $(LOCAL_INCLUDE) 

DEFS = -DHAVE_CONFIG_H
CFLAGS += $(DEFS)  

TARGET		= $(LIB_DIR)/libevent.a

LDFLAGS = -L$(LIB_DIR)/ -nostartfiles -L$(INSTALL_DIR)/lib -Ttarget.ld 

SOURCES =

SOURCES += 	buffer.c \
    bufferevent.c	\
    bufferevent_filter.c	\
    bufferevent_pair.c	\
    bufferevent_ratelim.c	\
    bufferevent_sock.c	\
    event.c	\
    evmap.c	\
    evthread.c	\
    evutil.c	\
    evutil_rand.c	\
    evutil_time.c	\
    listener.c	\
    log.c	\
    signal.c	\
    strlcpy.c	\
    select.c	\
    poll.c	\
    event_tagging.c	\
    http.c	\
    evdns.c	\
    evrpc.c	\
    epoll_sub.c \
    epoll.c
    
    

OBJS_NAME = $(basename $(SOURCES))

APP=N
include $(ROOT_DIR)/Makefile.in
