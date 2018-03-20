#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include "arena.h"

int robot_load( char *fname, ROBOT *r )
{
	FILE *f;
	
	bzero( r, sizeof(ROBOT) );

	f=fopen( fname, "rb" );
	if( f == NULL ) {
		fprintf( stderr, "Unable to open file '%s': (%d:%s)\n", fname, errno, strerror( errno ) );
		return( ARENA_ERR );
	}

	r->r_id=0;
	r->r_state=STATE_INIT;
	r->r_fuel=200;
	r->r_shield=SHIELD_OFF;

	r->r_size=fread( r->r_memory, 1, 65536, f );
	fprintf( stderr, "DEBUG: Loaded robot '%s' with %u bytes\n", fname, r->r_size );
	/* TODO - check for errors, did we read all of it, EINTR, etc */
	fclose( f );
	r->r_next=NULL;

	return( ARENA_OK );
}
