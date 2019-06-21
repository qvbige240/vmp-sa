/**
 * History:
 * ================================================================
 * 2019-06-20 qing.zou created
 *
 */

#ifndef VMP_PORTS_H
#define VMP_PORTS_H

#include "vpk.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DIGIT_SIZE				(0xFFFF+1)
#define MAX_DIGIT_MASK				((uint32_t)(0xFFFF))

#define VMP_DIGITAL_OUT_OF_RANGE	((uint32_t)(-1))
#define VMP_TAKEN_SINGLE			((uint32_t)(-2))

typedef struct vmp_digital_s
{
	uint32_t		status[MAX_DIGIT_SIZE];
	uint32_t		low;
	uint32_t		high;
	uint16_t		range_start;
	uint16_t		range_stop;
	uint16_t		ports[MAX_DIGIT_SIZE];	void			*mutex_lock;
} vmp_ports_t;


vmp_ports_t* vmp_ports_create(uint16_t start, uint16_t end);
void vmp_ports_destroy(vmp_ports_t *vp);
//int vmp_ports_init(vmp_ports_t *vp, uint16_t start, uint16_t end);
int vmp_ports_allocate(vmp_ports_t *vp);
void vmp_ports_release(vmp_ports_t *vp, uint16_t port);


#ifdef __cplusplus
}
#endif

#endif //VMP_PORTS_H
