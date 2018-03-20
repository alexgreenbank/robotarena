#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "arena.h"
#include "hash.h"
#include "instr.h"

void memory_set_uint( uint32_t pc, unsigned char *mem, uint32_t n )
{
	uint32_t *x;
	x=(uint32_t *)&mem[pc];
	*x=n;
}

void memory_set_float( uint32_t pc, unsigned char *mem, float n )
{
	float *x;
	x=(float *)&mem[pc];
	*x=n;
}

void memory_set_int( uint32_t pc, unsigned char *mem, int32_t n )
{
	int32_t *x;
	x=(int32_t *)&mem[pc];
	*x=n;
}

int main(int argc, char **argv )
{
	FILE *in,*out;
	int32_t itmp;
	uint32_t utmp;
	float ftmp;
	uint32_t pc;
	uint32_t pass,linenos;
	int32_t ok,parse_ok;
	INSTRUCTION instr;
	ROBOT r;

	/* TODO - parse command line args */

	hash_init();

	in=fopen( "../ras/dumb.ras", "rb" );
	if( !in )
	{
		fprintf( stderr, "ras: error: Unable to open file '%s' for reading\n",
			"dumb.ras" );
		return( 1 );
	}

	/* TODO - initialise stuff */

	bzero( &r, sizeof( ROBOT ) );

	/* Pass 1 */
	ok=1;
	pass=1;
	pc=0;
	linenos=0;
	while( 1 )
	{
		char linebuf[512];
		char *curr,*tmp;
		int dup_label;
		fgets( linebuf, 511, in );
		linenos++;
		if( feof( in ) ) {
			if( pass == 1 ) {
				if( ok == 0 ) {
					/* pass one failed, bail */
					break;
				} else {
					fseek( in, 0, SEEK_SET );
					pass=2;
					pc=0;
					linenos=0;
					continue;
				}
			} else {
				/* done both passes. woo hoo */
				break;
			}
		}
		/* strip cr/lf */
		tmp=strchr( linebuf, '\n' ); if( tmp ) *tmp='\0';
		tmp=strchr( linebuf, '\r' ); if( tmp ) *tmp='\0';
		/* strip comments */
		tmp=strchr( linebuf, '-' ); if( tmp ) *tmp='\0';
		/* parse line */
		/* skip whitespace */
		curr=linebuf;
		while( *curr && isspace( *curr ) ) {
			curr++;
		}
		if( strncmp( curr, "INT", 3 ) == 0 ) {
			curr+=3;
			while( *curr && !isdigit( *curr ) ) {
				curr++;
			}
			if( sscanf( curr, "%d", &itmp ) != 1 ) {
				fprintf( stderr, "ras: error: invalid integer declaration at line %d\n", linenos );
				ok=0;
			} else {
				memory_set_int( pc, r.r_memory, itmp );
			}
			pc+=4;
			continue;
		} else if( strncmp( curr, "FLOAT", 5 ) == 0 ) {
			curr+=5;
			while( *curr && !isdigit( *curr ) ) {
				curr++;
			}
			if( sscanf( curr, "%f", &ftmp ) != 1 ) {
				fprintf( stderr, "ras: error: invalid integer declaration at line %d\n", linenos );
				ok=0;
			} else {
				memory_set_float( pc, r.r_memory, ftmp );
			}
			pc+=4;
			continue;
		} else if( strncmp( curr, "WORD", 4 ) == 0 ) {
			curr+=4;
			while( *curr && !isdigit( *curr ) ) {
				curr++;
			}
			if( sscanf( curr, "%u", &utmp ) != 1 ) {
				fprintf( stderr, "ras: error: invalid WORD declaration at line %d\n", linenos );
				ok=0;
			} else {
				if( utmp == 0 || (pc+(4*utmp)) > 65536 ) {
					fprintf( stderr, "ras: error: invalid WORD declaration at line %d\n", linenos );
					ok=0;
				} else {
					pc+=utmp<<2;
				}
			}
			continue;
		} else if( strncmp( curr, "STRING", 6 ) == 0 ) {
			curr+=6;
			/* move up to first double quote */
			while( *curr && *curr != '\"' ) {
				curr++;
			}
			if( ! *curr ) {
				fprintf( stderr, "ras: warning: no quoted STRING found on line %d\n", linenos );
			} else {
				int escaped=0;
				char *tmp=curr+1;
				int f,t;

				f=0; t=0;
				while( tmp[f] ) {
					if( escaped ) {
						switch( tmp[f] ) {
							case 'n': tmp[t]='\n'; t++; break;
							case 'r': tmp[t]='\r'; t++; break;
							case 't': tmp[t]='\t'; t++; break;
							case '"': tmp[t]='"'; t++; break;
							case '\\': tmp[t]='\\'; t++; break;
							default:
								fprintf( stderr, "ras: warning: unknown escaped character '\\%c' on line %d\n", tmp[f], linenos );
								tmp[t]='\\'; t++;
								tmp[t]=tmp[f]; t++;
								break;
						}
						f++;
						escaped=0;
					} else if( tmp[f] == '\\' ) {
						/* don't copy to t */
						escaped=1;
						f++;
					} else if( tmp[f] == '"' ) {
						/* end of string */
						f++;
						break;
					} else {
						tmp[t]=tmp[f];
						t++; f++;
					}
				}
				tmp[t]='\0';
				strcpy( (char *)&(r.r_memory[pc]), tmp );
				if( escaped ) {
					fprintf( stderr, "ras: warning: trailing escape character on line %d\n", linenos );
				}
				/* skip remaining whitespace */
				while( tmp[f] && isspace( tmp[f] ) ) {	
					f++;
				}
				if( tmp[f] ) {
					fprintf( stderr, "ras: warning: possibly corrupt STRING declaration on line %d\n", linenos );
				}
				if( (t % 4 ) == 0 ) {
					pc+=t;	
				} else {
					pc+=t+4-(t%4);
				}
			}
			continue;
		}
		switch( *curr ) {
			case '#':
			case '-':
			case '\0':
				/* Comment or blank line - do nothing */
				break;
			case '.':
				/* label, only process on first pass */
				if( pass == 1 ) {
					/* skip . */
					curr++;
					/* skip any whitespace */
					while( *curr && isspace( *curr ) ) {
						curr++;
					}
					tmp=curr;
					while( *tmp &&
						( isalnum( *tmp ) || *tmp == '_' ) ) {
						tmp++;
					}
					if( *tmp ) {
						if( !isspace( *tmp ) ) {
							fprintf( stderr, "ras: warning: extra characters at end of label on line %u\n", linenos );
						}
						*tmp='\0';
					}
					if( strlen( curr ) == 0 ) {
						fprintf( stderr, "ras: error: invalid blank label name at line %u\n", linenos );
						ok=0;
					}
			
					if( strcmp( curr, "pc" ) == 0 ||
						strcmp( curr, "lr" ) == 0 ) {
						fprintf( stderr, "ras: error: invalid label name '%s' at line %u\n", curr, linenos );
						fprintf( stderr, "ras: info: labels cannot mimic register names\n" );
						ok=0;
					}
					if( ( curr[0] == 'r' || curr[0] == 'f' ) &&
						isdigit( curr[1] ) ) {
						fprintf( stderr, "ras: error: invalid label name '%s' at line %u\n", curr, linenos );
						fprintf( stderr, "ras: info: labels cannot mimic register names\n" );
						ok=0;
					}
						
					if( hash_exists( curr ) ) {
						fprintf( stderr, "ras: error: duplicate label name '%s' at line %u\n", curr, linenos );
						ok=0;
					} else {
						//fprintf( stderr, "ras: debug: Adding label '%s' line %u value=%u\n", curr, linenos, pc );
						hash_add( curr, pc );
					}
					break;
				}
				/* do nothing with label on second pass */
				break;
			default:
				/* must be an instruction */
				parse_ok=instr_parse( curr, &instr, pass );
				if( parse_ok != 1 ) {
					ok=0;
				} else {
					memory_set_uint( pc, r.r_memory, instr.i_image );
					pc+=4;
				}
				break;
		}
	}
	fclose( in );
	if( pass == 1 ) {
		fprintf( stderr, "ras: error: stopping due to previous errors.\n" );
		return( 4 );
	}
	if( ok == 0 ) {
		/* must have failed on pass 2 */
		fprintf( stderr, "ras: error: stopping due to previous errors.\n" );
		return( 5 );
	}

	fprintf( stderr, "ras: debug: ok for writing output.\n" );
	fprintf( stderr, "ras: debug: final size=%u bytes.\n", pc );

	out=fopen( "../rob/dumb.rob", "wb" );
	if( ! out )
	{
		fprintf( stderr, "ras: error: Unable to open file '%s' for writing\n",
			"dumb.rob" );
		return( 1 );
		
	}
	/* write out robot */
	fwrite( r.r_memory, pc, 1, out );
	/* check return code */
	fclose( out );
	return( 0 );
}
