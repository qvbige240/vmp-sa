/**
 * History:
 * ================================================================
 * 2017-05-08 qing.zou created
 *
 */
#ifndef TIMA_BUFFER_H
#define TIMA_BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _TimaBuffer
{
	char* buffer;

	size_t buffer_used;
	size_t buffer_total;
} TimaBuffer;

void tima_buffer_init(TimaBuffer* thiz, size_t size);
void tima_buffer_reset(TimaBuffer* thiz);
void tima_buffer_clean(TimaBuffer* thiz);
size_t tima_buffer_size(TimaBuffer* thiz);
size_t tima_buffer_used(TimaBuffer* thiz);
int tima_buffer_expand(TimaBuffer* thiz, size_t expand);
char* tima_buffer_strdup_ex(TimaBuffer* thiz, const char* data, size_t length);
char* tima_buffer_strdup(TimaBuffer* thiz, const char* data, size_t length, int append);
char* tima_buffer_data(TimaBuffer* thiz, size_t offset);

void tima_buffer_align(TimaBuffer* thiz, size_t offset);

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // TIMA_BUFFER_H
