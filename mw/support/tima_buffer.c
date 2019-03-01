/**
 * History:
 * ================================================================
 * 2017-05-08 qing.zou created
 *
 */	 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vmp.h"
#include "tima_typedef.h"
#include "tima_buffer.h"


void tima_buffer_init(TimaBuffer* thiz, size_t size)
{
	thiz->buffer = (char*)malloc(size+1);
	if (!thiz->buffer)
		TIMA_LOGE("malloc error!");

	memset(thiz->buffer, 0x00, size+1);
	thiz->buffer_used  = 0;
	thiz->buffer_total = size;
}

int tima_buffer_expand(TimaBuffer* thiz, size_t expand)
{
	size_t used = thiz->buffer_used;
	size_t total = thiz->buffer_total;
	size_t need = total + (total >> 1);

	expand += used;

	while (need < expand) {
		need = need + (need >> 1);
	}
	//TIMA_LOGD("total: %d, used: %d, need: %d, expand: %d", total, used, expand-used, need);
	char* buffer = realloc(thiz->buffer, need);
	if (!buffer) {
		TIMA_LOGE("realloc memory error!\n");
		return -1;
	}
	thiz->buffer = buffer;
	thiz->buffer_total = need;

	return 0;
}

char* tima_buffer_strdup_ex(TimaBuffer* thiz, const char* data, size_t length)
{
	int offset = thiz->buffer_used;

	if((offset + length) >= thiz->buffer_total)
	{
		if (tima_buffer_expand(thiz, length) == -1) {
			TIMA_LOGE("buffer expand failed, at used(%d) length(%d)!", offset, length);
			return NULL;
		}
	}

	//strncpy(thiz->buffer + offset, data, length);
	memcpy(thiz->buffer + offset, data, length);
	thiz->buffer[offset + length] = '\0';
	thiz->buffer_used += length;

	return (thiz->buffer + 0);
}

char* tima_buffer_strdup(TimaBuffer* thiz, const char* data, size_t length, int append)
{
	int offset = thiz->buffer_used;
	if (length <= 0) return append ? (thiz->buffer+0) : (thiz->buffer+offset);

	if((offset + length) >= thiz->buffer_total)
	{
		//TIMA_LOGD("need: %d, len: %d, data: %s", length, strlen(data), data);
		if (tima_buffer_expand(thiz, length) == -1) {
			TIMA_LOGE("buffer expand failed, at used(%d) length(%d)!", offset, length);
			return NULL;
		}
	}

	//strncpy(thiz->buffer + offset, data, length);
	memcpy(thiz->buffer + offset, data, length);
	thiz->buffer[offset + length] = '\0';
	//strtrim(thiz->buffer+offset);
	if (append) {
		thiz->buffer_used += length;
		return (thiz->buffer + 0);
	} else {
		thiz->buffer_used += length + 1;
		return (thiz->buffer + offset);
	}

	//thiz->buffer_used += length;

	//if((thiz->buffer_used + length) >= thiz->buffer_total)
	//{
	//	size_t t_length = thiz->buffer_total + (thiz->buffer_total >> 1) + 128;
	//	char* buffer = realloc(thiz->buffer, t_length);
	//	if(buffer != NULL)
	//	{
	//		thiz->buffer = buffer;
	//		thiz->buffer_total = t_length;
	//	}
	//}

	//if((thiz->buffer_used + length) >= thiz->buffer_total)
	//{
	//	return offset;
	//}

	//offset = thiz->buffer_used;
	//strncpy(thiz->buffer + offset, start, length);
	//thiz->buffer[offset + length] = '\0';
	//strtrim(thiz->buffer+offset);
	//thiz->buffer_used += length + 1;
}

void tima_buffer_align(TimaBuffer* thiz, size_t offset)
{
	memmove(thiz->buffer, thiz->buffer+offset, thiz->buffer_used-offset+1);
	thiz->buffer_used = thiz->buffer_used - offset;
}

char* tima_buffer_data(TimaBuffer* thiz, size_t offset)
{
	return thiz->buffer + offset;
}

size_t tima_buffer_used(TimaBuffer* thiz)
{
	return thiz->buffer_used;
}

size_t tima_buffer_size(TimaBuffer* thiz)
{
	return thiz->buffer_total;
}

void tima_buffer_reset(TimaBuffer* thiz)
{
	thiz->buffer_used = 0;
	memset(thiz->buffer, 0x00, thiz->buffer_total);
}

void tima_buffer_clean(TimaBuffer* thiz)
{
	if (thiz->buffer)
	{
		free(thiz->buffer);
		thiz->buffer = NULL;
	}
	thiz->buffer_total = 0;
}
