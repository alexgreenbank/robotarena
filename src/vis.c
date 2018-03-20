#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "arena.h"
#include <cairo/cairo.h>

/*
PSTATE: 1493
RSTATE: r=0 1493 pos=278,41 h=193 damage_ct=0 rot_ct=0 for_ct=0 health=6 fuel=200 shield=0
RSTATE: r=1 1493 pos=289,71 h=195 damage_ct=0 rot_ct=0 for_ct=0 health=9 fuel=97 shield=0
MSTATE: m=0x199e0e0 r=1 state=R pos=287,66 ang=195 range=17 ct=0
PSTATE_END
*/

ROBOT robots[MAX_ROBOTS];
MISSILE missiles[MAX_ROBOTS];
uint32_t r_ct;
uint32_t m_ct;
uint32_t pc_ct;

cairo_surface_t *surface;

void dopic( void )
{
	uint32_t loop;
	char fname[512];
	cairo_t *cr;
	cairo_text_extents_t extents;

	if( 0 && pc_ct != 1432 ) return;

	surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 640, 480);
	cr = cairo_create (surface);

	cairo_select_font_face (cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 8.0);

	cairo_set_line_width( cr, 1.0 );
	cairo_set_source_rgb( cr, 1.0, 1.0, 1.0 );
	cairo_rectangle( cr, 10, 10, ARENA_X, ARENA_Y );
	cairo_stroke( cr );

	loop=0;
	while( loop < r_ct ) {
		ROBOT *r=&robots[loop];
		if( r->r_state == STATE_RUN ) {
			cairo_arc( cr, (double)r->r_x, (double)r->r_y, 5.0, 0.0, 2*M_PI );
			if( r->r_damage_ct % 2 == 0 ) {
				cairo_stroke( cr );
			} else {
				cairo_fill( cr );
			}
			if( r->r_shield ) {
				cairo_arc( cr, (double)r->r_x, (double)r->r_y, 7.5, 0.0, 2*M_PI );
				cairo_stroke( cr );
			}
			cairo_move_to( cr, (double)r->r_x + (5.0*sin(r->r_head*2*M_PI/360)), (double)r->r_y + (5.0*cos(r->r_head*2*M_PI/360)) );
			cairo_line_to( cr, (double)r->r_x + (10.0*sin(r->r_head*2*M_PI/360)), (double)r->r_y + (10.0*cos(r->r_head*2*M_PI/360)) );
			cairo_stroke( cr );
			cairo_rectangle( cr, 20+ARENA_X, 10+(loop*90), 130, 80 );
			cairo_stroke( cr );

			sprintf( fname, "%u  %u,%u", loop, r->r_x, r->r_y );
			cairo_text_extents (cr, fname, &extents);

			cairo_move_to (cr, 20+ARENA_X+2, 10+(loop*90)+2+8 );
			cairo_show_text (cr, fname );

			sprintf( fname, "H: %3u  F: %u", r->r_head, r->r_fuel );
			cairo_text_extents (cr, fname, &extents);

			cairo_move_to (cr, 20+ARENA_X+2, 10+(loop*90)+2+8+10 );
			cairo_show_text (cr, fname );

			sprintf( fname, "Health: %u", r->r_health );
			cairo_text_extents (cr, fname, &extents);

			cairo_move_to (cr, 20+ARENA_X+2, 10+(loop*90)+2+8+10+10 );
			cairo_show_text (cr, fname );
			cairo_stroke( cr );
		}
		loop++;
	}
	loop=0;
	while( loop < m_ct ) {
		MISSILE *m=&missiles[loop];
		if( m->m_state == MISSILE_RUN ) {
			cairo_move_to( cr, (double)m->m_lx, (double)m->m_ly );
			cairo_line_to( cr, (double)m->m_lx + (5.0*sin(m->m_lang*2*M_PI/360)), (double)m->m_ly+(5.0*cos(m->m_lang*2*M_PI/360)) );
			cairo_stroke( cr );
		}	
		if( m->m_state == MISSILE_EXPLOSION ) {
			int32_t mrad=0;
			uint32_t ct=m->m_ct;
			if( ct <= 6 ) {
				mrad=(1+ct+ct);
			} else if( ct <= 12 ) {
				mrad=1+(12-ct)+(12-ct);
			}
			while( mrad >= 0 ) {
				cairo_arc( cr, (double)m->m_lx, (double)m->m_ly, (double)mrad, 0.0, 2*M_PI );
				cairo_stroke( cr );
				mrad-=2;
			}
			
		}
		loop++;
	}

	cairo_destroy( cr );
	sprintf( fname, "pngs/%.5u.png", pc_ct );
	cairo_surface_write_to_png( surface, fname );
	cairo_surface_destroy( surface );
}

void chomp( char *x )
{
	char *y;
	y=strchr( x, '\n' ); if( y ) *y=0;
	y=strchr( x, '\r' ); if( y ) *y=0;
}

int main(void)
{
	FILE *f=fopen( "0.vis", "rb" );
	if( !f ) {
		printf( "Unable to open file\n" );
		return(1);
	}
	pc_ct=0;
	while( 1 ) {
		char lbuf[512];
		lbuf[0]=0;
		fgets( lbuf, 511, f );
		if( feof( f ) ) {
			break;
		}
		chomp(lbuf);
		if( strncmp( lbuf, "PSTATE:", 7 ) == 0 ) {
			if( sscanf( lbuf, "PSTATE: %u", &pc_ct ) != 1 ) {
				printf( "ERROR: badly formatted PSTATE line; %s\n", lbuf );
				return(2);
			}
			r_ct=0;
			m_ct=0;
		} else if( strncmp( lbuf, "RSTATE: ", 7 ) == 0 ) {
			uint32_t cycles;
			if( strstr( lbuf, "DEAD" ) ) {
				if( sscanf( lbuf, "RSTATE: r=%u DEAD: %u", &robots[r_ct].r_id, &robots[r_ct].r_state ) != 2 ) {
					printf( "ERROR: badly formatted RSTATE line; %s\n", lbuf );
					return(2);
				}
				r_ct++;
			} else {
				if( sscanf( lbuf, "RSTATE: r=%u %u pos=%u,%u h=%u damage_ct=%u rot_ct=%u for_ct=%u health=%u fuel=%u shield=%u",
					&robots[r_ct].r_id,
					&cycles,
					&robots[r_ct].r_x,
					&robots[r_ct].r_y,
					&robots[r_ct].r_head,
					&robots[r_ct].r_damage_ct,
					&robots[r_ct].r_rot_ct,
					&robots[r_ct].r_for_ct,
					&robots[r_ct].r_health,
					&robots[r_ct].r_fuel,
					&robots[r_ct].r_shield ) != 11 ) {
						printf( "ERROR: badly formatted RSTATE line; %s\n", lbuf );
						return(2);
				}
				robots[r_ct].r_state=STATE_RUN;
				r_ct++;
			}
		} else if( strncmp( lbuf, "MSTATE: ", 7 ) == 0 ) {
			uint32_t dummy_ptr;
			uint32_t r_id;
			if( strstr( lbuf, " state=R " ) ) {
				if( sscanf( lbuf, "MSTATE: m=%x r=%u state=R pos=%u,%u ang=%u range=%u ct=%u",
					&dummy_ptr, &r_id, &missiles[m_ct].m_lx, &missiles[m_ct].m_ly, &missiles[m_ct].m_lang, &missiles[m_ct].m_range, &missiles[m_ct].m_ct ) != 7 ) {
					printf( "ERROR: badly formatted MSTATE R line; %s\n", lbuf );
					return(2);
				}
				missiles[m_ct].m_robot=(ROBOT *)r_id;	/* hacky TODO - do properly */
				missiles[m_ct].m_state=MISSILE_RUN;
				m_ct++;
			} else if( strstr( lbuf, " state=E " ) ) {
				if( sscanf( lbuf, "MSTATE: m=%x r=%u state=E pos=%u,%u ct=%u",
					&dummy_ptr, &r_id, &missiles[m_ct].m_lx, &missiles[m_ct].m_ly, &missiles[m_ct].m_ct ) != 5 ) {
					printf( "ERROR: badly formatted MSTATE E line; %s\n", lbuf );
					return(2);
				}
				missiles[m_ct].m_robot=(ROBOT *)r_id;	/* hacky TODO - do properly */
				missiles[m_ct].m_state=MISSILE_EXPLOSION;
				m_ct++;
			} else {
				printf( "UNHANDLED: MSTATE %s\n", lbuf );
				return(2);
			}
		} else if( strcmp( lbuf, "PSTATE_END" ) == 0 ) {
			dopic();
		} else {
			printf( "UNHANDLED: %s\n", lbuf );
		}
	}
	fclose(f);
	return(0);
}
