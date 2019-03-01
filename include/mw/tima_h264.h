/**
 * History:
 * ================================================================
 * 2019-02-28 qing.zou created
 *
 */
#ifndef TIMA_H264_H
#define TIMA_H264_H

#include "vmp.h"

TIMA_BEGIN_DELS

typedef struct h264_nalu_t {
	size_t		nalu_len;
	char		nalu_type;
	char*		nalu_data;
} h264_nalu_t;


typedef struct h264_meta_t
{
	int			size;		/** Metadata buffer length **/
	char		*data;		/** Metadata buffer **/
} h264_meta_t;


int h264_metadata_get(const char *buffer, size_t size, size_t offset, h264_meta_t *meta);

TIMA_END_DELS

#endif //TIMA_H264_H
