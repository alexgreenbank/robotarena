#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include "prng.h"

/* TODO - need a stable portable PRNG - Source from TAOCP? */

void prng_init( uint32_t n)
{
	srand( n );
}

uint32_t prng_rand( PRNG *p, uint32_t m )
{
	return( rand() % m );
}
