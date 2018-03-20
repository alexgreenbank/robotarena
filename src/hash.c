#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "arena.h"

typedef struct hash_node {
	struct hash_node *prev;
	struct hash_node *next;
	char *name;
	int32_t value;
} HASH_NODE;

HASH_NODE *hash_data[1024];

uint32_t hash_func( char *text )
{
	int ct;
	uint32_t n;
	n=0;
	ct=0;
	while( text[ct] ) {
		n+=text[ct];
		ct++;
	}
	return( n );
}

int32_t hash_exists( char *name )
{
	uint32_t val;
	int bucket;
	HASH_NODE *walk;

	val=hash_func( name );
	bucket = val % 1024;
	walk=hash_data[bucket];
	while( walk ) {
		if( strcmp( walk->name, name ) == 0 ) {
			return( 1 );
		}
		walk=walk->next;
	}
	return( 0 );
}

int32_t hash_get( char *name )
{
	uint32_t val;
	int bucket;
	HASH_NODE *walk;

	val=hash_func( name );
	bucket = val % 1024;
	walk=hash_data[bucket];
	while( walk ) {
		if( strcmp( walk->name, name ) == 0 ) {
			return( walk->value );
		}
		walk=walk->next;
	}
	return( 0 );
}

void hash_add( char *name, int32_t value )
{
	uint32_t val;
	int bucket;
	HASH_NODE *new;

	val=hash_func( name );
	bucket = val % 1024;
	new=malloc(sizeof(HASH_NODE));
	if( ! new ) {
		fprintf( stderr, "fatal: MALLOC ERROR in hash_add\n" );
		abort();
	}
	new->value=value;
	new->name=(char *)strdup( name );
	if( ! new->name ) {
		fprintf( stderr, "fatal: MALLOC ERROR in hash_add\n" );
		abort();
	}
	new->prev=NULL;
	new->next=hash_data[bucket];
	if( new->next != NULL ) {
		hash_data[bucket]->prev=new;
	}
	hash_data[bucket]=new;
}

void hash_init( void )
{
	bzero( hash_data, sizeof(HASH_NODE *) * 1024 );
}

void hash_delete( char *name )
{
	/* TODO - remove entry */
}

void hash_flush( void )
{
	/* TODO - remove all entries */
}
