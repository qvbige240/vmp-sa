CC=gcc
CFLAGS=-g -O0 -W -Wall
TARGET=g711_voice
#SRC=$(wildcard *.c)
SRC:=$(wildcard g711.c g711_table.c voice_8000_1.c)
#SRC=voice_8000_1.c g711.c g711_table.c

OBJECT= ${SRC:.c=.o}
	INCLUDES=-I/usr/include/alsa
	LDFLAGS=-lasound -lpthread

all:$(TARGET)

#$(OBJECT):$(SRC)
#	    $(CC) $(CFLAGS) -c $(INCLUDES) $<

$(OBJECT):$(SRC)
	    $(CC) $(CFLAGS) $(INCLUDES) -c $?

$(TARGET):$(OBJECT)
	    $(CC) -o $@ $? $(LDFLAGS)

.PHONY:clean

clean:
	    @rm -rf $(OBJECT) $(TARGET) *~
