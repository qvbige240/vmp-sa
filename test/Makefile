TOP ?= ..
NEW_APP_DIR=.

include $(TOP)/make.include

#MAIN_APP
SOURCES		+= tvmpss_client_main.c tvmpss_client.c

#---------------------------------------------------------------------------
# Internal Include dir (User)
#---------------------------------------------------------------------------
INCLUDE_DIR += \
	-I.\
	-I$(TOP)/include  \
	-I$(TOP)/include/common \
	-I$(TOP)/include/libevent \
	-I$(TOP)/include/libevent/internal \
	-I$(TOP)/include/threadpool/ 

APP=Y

TARGET = $(TOP)/bin/tvmpss_client

include $(TOP)/Makefile.in

