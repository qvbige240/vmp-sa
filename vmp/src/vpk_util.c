/**
 * History:
 * ================================================================
 * 2017-03-26 qing.zou created
 *
 */
 
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>

#include <arpa/inet.h>

#include "vpk_util.h"

int vpk_hex_to_int(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	else if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 0x0A;
	}
	else if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 0x0a;
	}

	return 0;
}

char* vpk_strstrip(char* str, char c)
{
	if (str == NULL)
		return str;

	{
		char* s = str;
		char* d = str;
		while (*s != '\0')
		{
			if (*s != c) 
			{
				*d = *s;
				d++;
			}
			s++;
		}
		*d = '\0';
	}

	return str;
}

char* remove_colons(char* str)
{
	char* p = NULL;
	if (str == NULL)
		return str;

	p = str + strlen(str) - 1;

	while(p != str && *p == ':')
	{
		*p = '\0';
		p--;
	}

	p = str;
	while(*p != '\0' && *p == ':') p++;

	//if (p != str)
	{
		char* s = p;
		char* d = str;
		while (*s != '\0')
		{
			if (*s != ':') 
			{
				*d = *s;
				d++;
			}
			s++;
		}
		*d = '\0';
	}

	return str;
}

int vpk_strcntstr(const char *s1, const char *s2)
{
	int cnt = 0;
	char *p = NULL, *ptr = (char*)s1;
	unsigned int len1 = 0, len2 = 0;

	len1 = strlen(s1);
	len2 = strlen(s2);

	if (!len2)
		return 0;
	
	while (s1 < ptr + len1)
	{
		p = strstr(s1, s2);
		if (p == NULL)
			break;

		cnt++;
		s1 = p + len2;
	}

	return cnt;
}

void vpk_snprintf(char *buf, unsigned int *pos, size_t len, const char *format, ...)
{
	va_list va;
	va_start(va, format);
	*pos += vsnprintf(&buf[*pos], len - *pos, format, va);
	va_end(va);
	if (*pos >= len ) {
		buf[len] = 0;
		*pos = len;
	}
}
/*
static char* strcat_arg(char* str, unsigned int len, const char* first, va_list arg)
{
	unsigned int dst = 0;
	const char* iter = first;
	if (len <= 0 || iter == NULL)
	{
		return NULL;
	}

	while(iter != NULL && dst < len)
	{
		for (; *iter != NULL && dst < len; dst++,iter++)
		{
			str[dst] = *iter;
		}

		iter = va_arg(arg, char*);
	}
	va_end(arg);

	if (dst < len)
	{
		str[dst] = '\0';
	}
	else
	{
		str[len-1] = '\0';
	}

	return str;
}

char* vpk_strcat(char* str, unsigned int len, const char* first, ...)
{
	va_list args;
	return_val_if_fail(str != NULL && len > 0, NULL);

	va_start(args, first);
	strcat_arg(str, len, first, args);
	//va_end(args);

	return str;
}*/

#ifndef VPK_HAVE_GETTIMEOFDAY
int vpk_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct _timeb tb;

	if (tv == NULL)
		return -1;

	_ftime(&tb);
	tv->tv_sec  = (long) tb.time;
	tv->tv_usec = ((int) tb.millitm) * 1000;
	return 0;
}
#endif

int vpk_socket_closeonexec(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFD, NULL)) < 0) {
		printf("fcntl(%d, F_GETFD)", fd);
		return -1;
	}
	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
		printf("fcntl(%d, F_SETFD)", fd);
		return -1;
	}

	return 0;
}

int vpk_socket_nonblocking(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
		printf("fcntl(%d, F_GETFL)", fd);
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		printf("fcntl(%d, F_GETFL)", fd);
		return -1;
	}

	return 0;
}

void* vpk_sockaddr_get_addr(const vpk_sockaddr *addr)
{
	const vpk_sockaddr *a = (const vpk_sockaddr*)addr;

	return_val_if_fail(a->ss.sa_family == AF_INET ||
		a->ss.sa_family == AF_INET6, NULL);

	if (a->ss.sa_family == AF_INET6)
		return (void*) &a->s6.sin6_addr;
	else
		return (void*) &a->s4.sin_addr;
}

unsigned short vpk_sockaddr_get_port(const vpk_sockaddr *addr)
{
	const vpk_sockaddr *a = (const vpk_sockaddr*) addr;

	return_val_if_fail(a->ss.sa_family == AF_INET ||
		a->ss.sa_family == AF_INET6, (unsigned short)0xFFFF);

	return ntohs((unsigned short)(a->ss.sa_family == AF_INET6 ?
		a->s6.sin6_port : a->s4.sin_port));
}

int vpk_inet_ntop(int af, const void *src, char *dst, int size)
{
	return_val_if_fail(src && dst && size, -1);

	*dst = '\0';

	return_val_if_fail(af==AF_INET || af==AF_INET6, -2);

	if (inet_ntop(af, src, dst, size) == NULL) {
		int status = errno;
		if (status == 0)
			status = -3;

		return status;
	}

	return 0;
}

#if 0
int main(int argc, char* argv[])
{
	int cnt_num = 0;
	const char *ptr = "/dev/ttyUSB0  /dev/ttyUSB1  /dev/ttyUSB2  /dev/ttyUSB3";

	cnt_num = vpk_strcntstr(ptr, "ttyUSB");
	printf("cnt_num = %d\n", cnt_num);

	return 0;
}
#endif


