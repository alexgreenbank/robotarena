/*
 * Parse args and define arena
 * Load robots
 * Execute
 * Report results
 */

#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "rob.h"
#include "cpu.h"

/* void arena_exec( uint32_t seed, uint32_t nosrobs, ROBOT **robots ) */

ROBOT robots[MAX_ROBOTS];

int main( int argc, char **argv )
{
	uint32_t r;
	uint32_t seed;
	uint32_t w0,w1,draw;

	r=robot_load( "../rob/dumb.rob", &robots[0] );
	/* TODO - error check */
	r=robot_load( "../rob/bully.rob", &robots[1] );
	/* TODO - error check */

	if( 0 ) {
		w0=0; w1=0; draw=0;
		seed=0;
		while( seed < 1000 ) {
			arena_exec( seed, 2, robots );
			if( robots[0].r_pc > robots[1].r_pc ) {
				w0++;
			} else if( robots[0].r_pc < robots[1].r_pc ) {
				w1++;
			} else {
				draw++;
			}
			seed++;
		}

		printf( "FINAL: w0=%u w1=%u draw=%u\n", w0, w1, draw );
	} else {
		arena_exec( 0, 2, robots );
	}
	return( 0 );
}
