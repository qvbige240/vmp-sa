CC=gcc
CFLAGS=-g -O0 -W -Wall
TARGET=g711_voice.demo
#SRC=$(wildcard *.c)
SRC:=$(wildcard g711.c g711_table.c voice_8000_1.c)
#SRC=voice_8000_1.c g711.c g711_table.c

TARGET_DEMO1=remote_play.demo
SRC1:=$(wildcard g711.c g711_table.c remote_play.c)

OBJECT= ${SRC:.c=.o}
	INCLUDES=-I/usr/include/alsa
	LDFLAGS=-lasound -lpthread

OBJECT1= ${SRC1:.c=.o}
	INCLUDES=-I/usr/include/alsa
	LDFLAGS=-lasound -lpthread

all:$(TARGET) $(TARGET_DEMO1)

#$(OBJECT):$(SRC)
#	    $(CC) $(CFLAGS) -c $(INCLUDES) $<

$(OBJECT):$(SRC)
	    $(CC) $(CFLAGS) $(INCLUDES) -c $?

$(TARGET):$(OBJECT)
	    $(CC) -o $@ $? $(LDFLAGS)

$(OBJECT1):$(SRC1)
	    $(CC) $(CFLAGS) $(INCLUDES) -c $?

$(TARGET_DEMO1):$(OBJECT1)
	    $(CC) -o $@ $? $(LDFLAGS)

.PHONY:clean

clean:
	    @rm -rf $(OBJECT) $(TARGET) $(OBJECT1) $(TARGET_DEMO1) *~
