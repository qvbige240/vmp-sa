TOP ?= ..
NEW_APP_DIR=.

include $(TOP)/make.include

#MAIN_APP
SOURCES		+= main.c

#---------------------------------------------------------------------------
# Internal Include dir (User)
#---------------------------------------------------------------------------
INCLUDE_DIR += \
	-I.\
	-I$(TOP)/include  \
	-I$(TOP)/include/common \
	-I$(TOP)/include/libevent \
	-I$(TOP)/include/libevent/internal \
	-I$(TOP)/include/threadpool \
	-I$(TOP)/include/bll/ \

APP=Y

TARGET = $(TOP)/bin/tvmpssd

#BIN_DIR = $(TOP)/bin

include $(TOP)/Makefile.in

