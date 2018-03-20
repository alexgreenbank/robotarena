#include <stdio.h>
typedef unsigned int uint32_t;

int main(void)
{
	printf( "float is %d bytes.\n", sizeof(float));
	printf( "uint32_t is %d bytes.\n", sizeof(uint32_t));
	return( 0 );
}
