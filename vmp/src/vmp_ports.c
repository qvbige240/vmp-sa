/**
 * History:
 * ================================================================
 * 2019-06-20 qing.zou created
 *
 */

#include "vmp_ports.h"

#include "vmp_log.h"
#include "vmp_thread.h"

//test
//#define PORT_DEBUG	0


int vmp_ports_init(vmp_ports_t *vp, uint16_t start, uint16_t end);

static int port_is_taken(uint32_t status)
{
	int ret = -1;
	switch (status) {
	case VMP_TAKEN_SINGLE :
		ret = 1;
		break;
	default:
		ret = 0;
	};
	return ret;
}

static void vmp_ports_randomize(vmp_ports_t *vp)
{
	if (!vp) {
		VMP_LOGE("null point");
		return;
	}

	unsigned int size = (unsigned int)(vp->high - vp->low);
	unsigned int i = 0;
	unsigned int cycles = size * 10;
	for (i = 0; i < cycles; i++)
	{
		uint16_t port1 = (uint16_t)(vp->low + (uint16_t)(((unsigned long)random()) % ((unsigned long)size)));
		uint16_t port2 = (uint16_t)(vp->low + (uint16_t)(((unsigned long)random()) % ((unsigned long)size)));
		if (port1 != port2)
		{
			int pos1 = vp->status[port1];
			int pos2 = vp->status[port2];

#ifdef PORT_DEBUG
			printf(" %d[%d,%d] ", port1, vp->status[port1], vp->ports[port1]);
			printf("%d[%d,%d]    ", port2, vp->status[port2], vp->ports[port2]);
			printf("%d[%d,%d] ", pos1, vp->status[pos1], vp->ports[pos1]);
			printf("%d[%d,%d]    \n", pos2, vp->status[pos2], vp->ports[pos2]);
#endif // PORT_DEBUG

			int tmp = (int)vp->status[port1];
			vp->status[port1] = vp->status[port2];
			vp->status[port2] = (uint32_t)tmp;
			tmp = (int)vp->ports[pos1];
			vp->ports[pos1] = vp->ports[pos2];
			vp->ports[pos2] = (uint16_t)tmp;

#ifdef PORT_DEBUG			
			printf(" ->");
			printf(" %d[%d,%d] ", port1, vp->status[port1], vp->ports[port1]);
			printf("%d[%d,%d]    ", port2, vp->status[port2], vp->ports[port2]);
			printf("%d[%d,%d] ", pos1, vp->status[pos1], vp->ports[pos1]);
			printf("%d[%d,%d]    \n", pos2, vp->status[pos2], vp->ports[pos2]);
#endif // PORT_DEBUG
		}
	}
} 

#ifdef PORT_DEBUG	
static void test_ports_print(vmp_ports_t *vp)
{
	int i = 0;
	printf(" low high (%d,%d) \n ", vp->low, vp->high);
	//for (i = vp->range_start; i < vp->range_stop + 2; i++)
	for (i = vp->range_start; i < vp->range_stop + 1; i++)
	{
		printf(" %d[%d,%d] %s", i, vp->status[i], vp->ports[i], (i + 1 - vp->range_start) % 8 ? " " : "\n ");
	}

	if (vp->high > MAX_DIGIT_MASK+1)
	{
		printf("  |\n ");
		for (i = vp->range_stop+1; i < MAX_DIGIT_MASK+1; i++)
		{
			printf(" %d[%d,%d] %s", i, vp->status[i], vp->ports[i], (i-vp->range_stop) % 8 ? " " : "\n ");			
		}

		printf("\n ");
		for (i = 0; i < (vp->high & MAX_DIGIT_MASK); i++)
		{
			printf(" %d[%d,%d] %s", i, vp->status[i], vp->ports[i], (i + 1) % 8 ? " " : "\n ");			
		}
	} 
	else if (vp->high > vp->range_stop+1)
	{
		printf("  |\n ");
		for (i = vp->range_stop+1; i < vp->high; i++)
		{
			printf(" %d[%d,%d] %s", i, vp->status[i], vp->ports[i], (i-vp->range_stop) % 8 ? " " : "\n ");			
		}
	}

	printf("\n\n");
}
#endif // PORT_DEBUG

vmp_ports_t* vmp_ports_create(uint16_t start, uint16_t end)
{
	if (start > end) return NULL;

	vmp_ports_t* vp = (vmp_ports_t*)VPK_CALLOC(1, sizeof(vmp_ports_t));
	if (vp)
	{
		int ret = vmp_ports_init(vp, start, end);
		if (ret < 0) {
			VPK_FREE(vp);
			return NULL;
		}
	}

	return vp;
}

void vmp_ports_destroy(vmp_ports_t *vp)
{
	if (vp)
	{
		if (vp->mutex_lock) {
			vmp_thread_lock_free(vp->mutex_lock, 0);
			vp->mutex_lock = NULL;
		}

		VPK_FREE(vp);
	}
}

int vmp_ports_init(vmp_ports_t *vp, uint16_t start, uint16_t end)
{
	int i = 0;
	return_val_if_fail(vp, -1);

	if (end >= MAX_DIGIT_SIZE) {
		VMP_LOGE("===== end error: out of max digital scope ====\n");
		return -1;
	}

	vp->low		= start;
	vp->high	= ((uint32_t)end)+1;

	vp->range_start		= start;
	vp->range_stop		= end;

	for (i = 0; i < start; i++) {
		vp->status[i]	= VMP_DIGITAL_OUT_OF_RANGE;
		vp->ports[i]	= (uint16_t)i;
	}
	for (i = start; i <= end; i++) {
		vp->status[i]	= (uint32_t)i;
		vp->ports[i]	= (uint16_t)i;
	}
	for (i = ((int)end)+1; i < MAX_DIGIT_SIZE; i++) {
		vp->status[i]	= VMP_DIGITAL_OUT_OF_RANGE;
		vp->ports[i]	= (uint16_t)i;
	}

	vmp_ports_randomize(vp);

	VMP_THREAD_SETUP_GLOBAL_LOCK(vp->mutex_lock, VMP_THREAD_LOCKTYPE_RECURSIVE);

	return 0;
}

int vmp_ports_allocate(vmp_ports_t *vp)
{
	int port = -1;
	return_val_if_fail(vp, -1);

	VMP_MUTEX_LOCK(vp->mutex_lock, 0);

	while(1)
	{
		if (vp->high <= vp->low) {
			VMP_LOGE("port allocate, high <= low");
			VMP_MUTEX_UNLOCK(vp->mutex_lock, 0);
			return -1;
		}

		int position = (uint16_t)(vp->low & MAX_DIGIT_MASK);

		port = (int)vp->ports[position];
		if (port < (int)(vp->range_start) || port > ((int)(vp->range_stop))) {
			VMP_LOGE("port allocate, out [range_start, range_stop]");
			VMP_MUTEX_UNLOCK(vp->mutex_lock, 0);
			return -1;
		}

		if (port_is_taken(vp->status[port])) {
			++(vp->low);
			continue;
		}

		if (vp->status[port] != vp->low) {
			++(vp->low);
			continue;
		}

		vp->status[port] = VMP_TAKEN_SINGLE;
		++(vp->low);
		break;
	}

	VMP_MUTEX_UNLOCK(vp->mutex_lock, 0);

	return port;
}

void vmp_ports_release(vmp_ports_t *vp, uint16_t port)
{
	return_if_fail(vp && port > 0);

	VMP_MUTEX_LOCK(vp->mutex_lock, 0);

	if(vp && port >= vp->range_start && port <= vp->range_stop)
	{
		uint16_t position=(uint16_t)(vp->high & MAX_DIGIT_MASK);
		if (port_is_taken(vp->status[port])) {
			vp->status[port] = vp->high;
			vp->ports[position] = port;
			++(vp->high);
		}
	}

	VMP_MUTEX_UNLOCK(vp->mutex_lock, 0);
}
