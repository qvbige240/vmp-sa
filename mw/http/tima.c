/**
 * History:
 * ================================================================
 * 2019-03-15 wison.wei created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tima.h"

void tima_init(void)
{
	tima_token_init();
	tima_get_property_init();
}

void tima_done(void)
{
	tima_token_done();
	tima_get_property_done();
}
