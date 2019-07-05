/**
 * History:
 * ================================================================
 * 2019-03-15 qing.zou created
 *
 */

#ifndef TIMA_JT1078_PARSER_H
#define TIMA_JT1078_PARSER_H

#include "vmp.h"

TIMA_BEGIN_DELS


typedef struct _stream_header
{
	unsigned char		magic[4];		/* 0x30 0x31 0x63 0x64, 4 byte */
	unsigned char		rtp;			/* 1 byte */
	unsigned char		pt;				/* 1 byte */
	unsigned short		flowid;			/* packet id 2 byte */
	unsigned char		simbcd[6];		/* sim number bcd code, 6 byte */
	unsigned char		channel;		/* channel id, 1 byte */
	unsigned char		mtype;			/* media data type, 1 byte */
	unsigned char		timestamp[8];	/* data type, 8 byte */
	unsigned short		lifinterval;	/* last I frame interval, 2 byte */
	unsigned short		lfinterval;		/* last frame interval, 2 byte */
	unsigned short		bodylen;		/* data body length, 2 byte */

	unsigned long long simno;
} stream_header_t;

int packet_jt1078_parse(unsigned char *packet, int length, stream_header_t *head, unsigned char **body);

/** customized msg **/
int jt1078_make_end(char *buffer, const char *data, unsigned short len);

TIMA_END_DELS

#endif //TIMA_JT1078_PARSER_H
