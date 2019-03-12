
#ifndef TM_AUDIO_TRANCODE_H
#define TM_AUDIO_TRANCODE_H

#include <stdio.h>
#include <ctype.h>  
#include <errno.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <faac.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    unsigned char *pb_acc_buf;
    int size;
}acc_packet;

typedef acc_packet* p_acc_packet;

p_acc_packet g711a2aac(unsigned char *pbG711ABuffer, int len);

#ifdef __cplusplus
}
#endif

#endif // TM_AUDIO_TRANCODE_H
