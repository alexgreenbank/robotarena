#include <sys/types.h>

typedef struct prng {
	int cval;
} PRNG;

extern void prng_init( uint32_t );
extern uint32_t prng_rand( PRNG *, uint32_t );
