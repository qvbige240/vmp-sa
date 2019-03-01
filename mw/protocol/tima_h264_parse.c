/**
 * History:
 * ================================================================
 * 2019-02-28 qing.zou created
 *
 */

#include <stdio.h>

#include "tima_h264.h"


static inline size_t find_start_code(const char *buf)
{
	if (buf[0] != 0x00 || buf[1] != 0x00 ) {
		return 0;
	}

	if (buf[2] == 0x01) {
		return 3;
	} else if (buf[2] == 0x00 && buf[3] == 0x01) {
		return 4;
	}

	return 0;
}

static int nalu_read(const char *buffer, size_t size, size_t offset, h264_nalu_t *nalu)
{
	size_t start = 0;
	if (offset < size) {
		size_t pos = 0;
		while (offset + pos + 3 < size) {
			start = find_start_code(buffer + offset + pos);
			if (start)
				break;

			pos++;
		}

		if (start == 0) {
			VMP_LOGW("can't find 00 00 01");
			return 0;
		}

		if(offset + pos + start == size){
			nalu->nalu_len = pos + start;
		} else {
			nalu->nalu_len = pos;
		}

		nalu->nalu_data = (char*)(buffer + offset);
		//nalu->forbidden_bit = nalu->nalu_data[0] & 0x80;
		//nalu->nal_reference_idc = nalu->nalu_data[0] & 0x60; // 2 bit
		nalu->nalu_type = (nalu->nalu_data[0]) & 0x1f;// 5 bit

		return (nalu->nalu_len + start);
	}

	return 0;
}

int h264_metadata_get(const char *buffer, size_t size, size_t offset, h264_meta_t *meta)
{
	size_t pos = offset, len = 0;
	h264_nalu_t sps, pps;

	len = find_start_code(buffer + pos);
	if (len == 0) {
		VMP_LOGE("h264 meta header format error: %02x %02x %02x %02x.", 
		buffer[offset+0], buffer[offset+1], buffer[offset+2], buffer[offset+3]);
		return -1;
	}

	pos = offset + len;
	
	len = nalu_read(buffer, size, pos, &sps);
	pos += len;
	len = nalu_read(buffer, size, pos, &pps);
	pos += len;
	if (meta) {
		meta->size = sps.nalu_len + pps.nalu_len + 8;
		meta->data = calloc(1, meta->size);
		if (!meta->data) {
			VMP_LOGE("calloc error.");
			return -1;
		}

		if (sps.nalu_type == 0x07) {
			meta->data[2] = (sps.nalu_len >> 8) & 0xff;
			meta->data[3] = sps.nalu_len & 0xff;
			memcpy(meta->data + 4, sps.nalu_data, sps.nalu_len);
		}

		if (pps.nalu_type == 0x08) {
			meta->data[sps.nalu_len + 6] = (pps.nalu_len >> 8) & 0xff;
			meta->data[sps.nalu_len + 7] = pps.nalu_len & 0xff;
			memcpy(meta->data + sps.nalu_len + 8, pps.nalu_data, pps.nalu_len);
		}
		VMP_LOGI("metadata size: %d", meta->size);
	}

	return 0;
}
