/**
 * History:
 * ================================================================
 * 2019-03-15 qing.zou created
 *
 */

#include "tima_jt1078_parser.h"

#define BCD_PARSE(c)		(((c & 0xf0) >> 4) * 10 + (c & 0x0f))

static unsigned long long simno_get(unsigned char *bcd)
{
	unsigned long long num = 0;
	unsigned char *ptr = bcd;
	int i = 0;
	//printf("bcd: ");
	for (i = 0; i < 6; i++)
	{
		unsigned digital = BCD_PARSE(*(ptr+i));
		num = num * 100 + digital;
	}
	//printf("\nparsed: %lld\n", num);

	return num;
}

/* 0x30 0x31 0x63 0x64 */
static inline size_t find_magic_code(const unsigned char *buf)
{
	if (buf[0] != 0x30 || buf[1] != 0x31 || buf[2] != 0x63 || buf[3] != 0x64) {
		int ii = 0;
		for (ii = 0; ii < 32; ii++)
		{
			printf("%02x ", buf[ii]);
		}
		printf("... data packet exception\n");
		return 0;
	}

	return 1;
}

/**
 * Parse jt1078 stream packet.
 *
 * @param packet	The socket stream.
 *
 * @return	0 on success
 *			-1 packet magic error.
 *			
 */
int packet_jt1078_parse(unsigned char *packet, int length, stream_header_t *head, unsigned char **body)
{
	int pos = 0;

	if (!find_magic_code(packet))
		return -1;

	memcpy(head->magic, packet, 4);
	pos += 4;
	head->rtp	= packet[pos++];
	head->pt	= packet[pos++];

	head->simno = simno_get(packet+8);
	//...

	head->channel	= packet[14];
	head->mtype		= packet[15];

	//int is_audio = ((packet[15]& 0xf0) == 0x30);
	//if (is_audio)
	//	head->bodylen = packet[28] * 2;
	//else
		head->bodylen = (packet[28] << 8) | packet[29];

	*body = packet + 30;

	if (head->bodylen > length - 30)	/* less than one packet */
		return 0;

	return head->bodylen + 30;	
}

/**
 * customized msg.
 *
 * @param buffer	The socket stream.
 * @param data		The "END".
 *
 * @return	0 on success
 *			
 */
int jt1078_make_end(char *buffer, const char *data, unsigned short len)
{
	int i = 0;

	char *p = buffer;
	p[i++] = 0x30;
	p[i++] = 0x31;
	p[i++] = 0x63;
	p[i++] = 0x64;

	p[14] = 0xff;
	p[15] = 0xff;

	p[28] = (len >> 8) & 0xff;
	p[29] = len & 0xff;

	memcpy(p+30, data, len);
	return 30+len+1;
}
