/*
 * Parse args and define arena
 * Load robots
 * Execute
 * Report results
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <strings.h>
#include "arena.h"
#include "prng.h"
#include "instr.h"

int pf=1;
int pf_vis=1;

MISSILE *missiles=NULL;

double dist_between_points( uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2 )
{
	return( hypot( (double)(x1-x2), (double)(y1-y2) ) );
}

double dist_intersection_line_circle( double lx, double ly, uint32_t lang, uint32_t llen, double px, double py, double pradius)
/*
 * Find shortest distance along lx,ly until coincides with a circle at px,py of radius pradius
 *
 * http://mathworld.wolfram.com/Circle-LineIntersection.html
 */
{
	double dx,dy,dr,D,delta,deltas;
	double x1,y1,x2,y2;
	double px1,px2,py1,py2;
	double pd1,pd2;
	double dang,dlen;
	double sgn;
	uint32_t dbg=0;
	dlen=(double)llen;
	dang=(double)lang;
	dang/=360;
	dang*=(2.0*3.14159265);

	/* Rebase as circle at 0,0 */
	x1=lx;
	y1=ly;
	x1-=px;
	y1-=py;
	x2=x1+(sin(dang)*dlen);
	y2=y1+(cos(dang)*dlen);
	D=(x1*y2)-(x2*y1);
	dx=x2-x1;
	dy=y2-y1;
	dr=hypot( dx, dy );

	delta=(pradius*pradius*dr*dr)-(D*D);
if( dbg ) {
	if( pf ) printf( "llen=%u\n", llen );
	if( pf ) printf( "dlen=%f\n", dlen );
	if( pf ) printf( "dang=%f\n", dang );
	if( pf ) printf( "pradius=%f\n", pradius );
	if( pf ) printf( "x1,y1 = %f,%f\n", x1, y1 );
	if( pf ) printf( "x2,y2 = %f,%f\n", x2, y2 );
	if( pf ) printf( "D = %f\n", D );
	if( pf ) printf( "dx = %f\n", dx );
	if( pf ) printf( "dy = %f\n", dy );
	if( pf ) printf( "dr = %f\n", dr );
	if( pf ) printf( "delta=%f\n", delta );
}
	if( delta < 0 ) {
		return( -999.0 );
	}
	deltas=sqrt(delta);
	sgn=1; if( dy < 0 ) { sgn=-1; }
	px1=((D*dy)+(sgn*dx*deltas))/(dr*dr);
	px2=((D*dy)-(sgn*dx*deltas))/(dr*dr);
	py1=((-1.0*D*dx)+(fabs(dy)*deltas))/(dr*dr);
	py2=((-1.0*D*dx)-(fabs(dy)*deltas))/(dr*dr);
if( dbg ) {
	if( pf ) printf( "p1 = %f,%f\n", px1, py1 );
	if( pf ) printf( "p2 = %f,%f\n", px2, py2 );
}

	/* Find which is closer */
	if( fabs( sin(dang) ) > fabs( cos(dang) ) ) {
		/* Use x diffs */
		pd1=(px1-x1)/sin(dang);
		pd2=(px2-x1)/sin(dang);
	} else {
		/* Use y diffs */
		pd1=(py1-y1)/cos(dang);
		pd2=(py2-y1)/cos(dang);
	}
if( dbg ) {
	if( pf ) printf( "pd1 = %f\n", pd1 );
	if( pf ) printf( "pd2 = %f\n", pd2 );
}
	if( pd1 < 0.0 || pd1 > dlen ) { pd1=-999.0; }
	if( pd2 < 0.0 || pd2 > dlen ) { pd2=-999.0; }

	if( pd1 < 0 && pd2 < 0 ) {
		return( -999.0 );
	}
	if( pd1 < pd2 ) {
		return( pd1 );
	}
	return( pd2 );
}

double det( double a, double b, double c, double d )
{
	return( (a*d)-(b*c) );
}

double dist_intersection_lines( double sx1, double sy1, uint32_t sang, double lx1, double ly1, uint32_t lang )
/*
 * Find how far going from sx,sy at ang until we get to a point on lx,ly/lang
 * 
 * http://mathworld.wolfram.com/Line-LineIntersection.html
 */
{
	uint32_t dbg=0;
	double sx2,sy2;
	double lx2,ly2;
	double dsang,dlang;
	double D1,D2,D3,D4;
	double px,py,pd;
	dsang=(double)sang;
	dsang/=360; dsang *= (2.0*3.14159265);
	dlang=(double)lang;
	dlang/=360; dlang *= (2.0*3.14159265);

	sx2=sx1;
	sy2=sy1;
	sx2+=1000*sin(dsang);
	sy2+=1000*cos(dsang);

	lx2=lx1;
	ly2=ly1;
	lx2+=1000*sin(dlang);
	ly2+=1000*cos(dlang);

if( dbg ) {
	if( pf ) printf( "s: %f,%f to %f,%f\n", sx1, sy1, sx2, sy2 );
	if( pf ) printf( "l: %f,%f to %f,%f\n", lx1, ly1, lx2, ly2 );
}

	D1=det( sx1, sy1, sx2, sy2 );
	D2=det( lx1, ly1, lx2, ly2 );
if( dbg ) {
	if( pf ) printf( "D1=%f\n", D1 );
	if( pf ) printf( "D2=%f\n", D2 );
}
	D3=det( sx1-sx2, sy1-sy2, lx1-lx2, ly1-ly2 );

if( dbg ) {
	if( pf ) printf( "D3=%f\n", D3 );
}

	if( fabs(D3) < 0.0000001 ) {
		return( -999.0 );
	}

	px=det( D1, sx1-sx2, D2, lx1-lx2 ) / D3;
	py=det( D1, sy1-sy2, D2, ly1-ly2 ) / D3;

if( dbg ) {
	if( pf ) printf( "px,py = %f,%f\n", px, py );
}

	if( fabs( sin(dsang) ) > fabs( cos(dsang) ) ) {
		/* Use x diffs */
		pd=(px-sx1)/sin(dsang);
	} else {
		/* Use y diffs */
		pd=(py-sy1)/cos(dsang);
	}

if( dbg ) {
	if( pf ) printf( "pd = %f\n", pd );
}

	if( pd < 0 ) {
		return( -999.0 );
	}
	return( pd );
}

void robot_pos( ROBOT *r, double *x, double *y )
{
	double ang_rad,dx,dy;
	if( r->r_for_ct == 0 ) {
		*x=(double)r->r_x;
		*y=(double)r->r_y;
		return;
	}
	/* moving */
	/* get original position */
	*x=(double)r->r_x;
	*y=(double)r->r_y;

	/* get bearing */
	ang_rad=(double)r->r_head;
	ang_rad/=360;
	ang_rad*=(2.0*3.14159265);

	/* get additional delta */
	dx=(double)r->r_for_ct;
	dy=(double)r->r_for_ct;
	dx*=sin(ang_rad);
	dy*=cos(ang_rad);

	/* Add on deltas */
	*x+=dx;
	*y+=dy;
}

int arena_overlapcheck( uint32_t x, uint32_t y, uint32_t nos, ROBOT *robots )
/*
 * Check previous robots to make sure robot does not overlap with any others
 */
{
	double d;
	uint32_t n=0;
	while( n < nos ) {
		/* check distance between this x,y and robot n's x,y */
		d=dist_between_points( x, y, robots[n].r_x, robots[n].r_y );
		if( d < 5.0 ) {
			if( pf ) printf( "Clash! %d,%d and %d,%d gives %lf\n", x, y, robots[n].r_x, robots[n].r_y, d ); /* TODO - remove this */
			return( ARENA_ERR );
		}
		n++;
	}
	return( ARENA_OK );
}

void missile_free( MISSILE *m )
{
	ROBOT *r;
	r=m->m_robot;
	if( r ) {
		r->r_missile=NULL;
		m->m_robot=NULL;
	}
}

void missile_laser( ROBOT *r, MISSILE *m, uint32_t pc_ct )
{
	/* No bang */
	m->m_state=MISSILE_DEAD;
}

void missile_new( ROBOT *r, int32_t range, uint32_t cycle )
{
	MISSILE *m;
	if( r->r_missile != NULL ) {
		if( pf ) printf( "DEBUG: robot r=%u attempting to launch missile when already has one\n", r->r_id );
		return;
	}
	m=malloc(sizeof(MISSILE));
	if( m == NULL ) {
		fprintf( stderr, "FATAL: Out of memory creating missile.\n" );
		abort();
	}
	range-=10;
	if( range < 0 ) {
		if( pf ) printf( "DEBUG: missile launched at sub 0 range by robot r=%u\n", r->r_id );
		range=0;
	}
	/* TODOVIS - missile launch */
	m->m_robot=r;
	r->r_missile=m;
	m->m_lx=r->r_x;
	m->m_ly=r->r_y;
	m->m_lang=r->r_head;
	m->m_range=range;
	m->m_state=MISSILE_RUN;
	m->m_ct=0;
	m->m_next=missiles;
	missiles=m;
	if( pf ) printf( "DEBUG: missile launch %u,%u head=%u range=%u\n", m->m_lx, m->m_ly, m->m_lang, range );
}

void robot_stop( ROBOT *r )
{
	r->r_state=STATE_STOP;
}

void robot_abend( ROBOT *r )
{
	r->r_state=STATE_ABEND;
}

void robot_dead( ROBOT *r, uint32_t cycle )
{
	r->r_state=STATE_DEAD;
	r->r_pc=cycle;
}

void robot_damage( ROBOT *r )
{
	if( r->r_damage_ct > 0 ) {
		/* Robot already being damaged don't do anything */
		return;
	}
	r->r_damage_ct = DAMAGE_CT;
	r->r_health--;
	/* if robot was turning then end that instruction */
	if( r->r_rot_ct != 0 ) {
		if( pf ) printf( "DEBUG: robot r=%u damage whilst ROT r_rot_ct=%d\n", r->r_id, r->r_rot_ct );
		r->r_rot_ct=0;
		r->r_regi[REG_PC]+=4; /* TODOVIS - only turned part of the way */
	}
	/* if robot was moving then end that instruction */
	if( r->r_for_ct != 0 ) {
		double x,y;
		if( pf ) printf( "DEBUG: robot r=%u damage whilst FOR r_for_ct=%d\n", r->r_id, r->r_for_ct );
		r->r_for_ct=0;
		r->r_for_dist=0;
		r->r_regi[REG_PC]+=4; /* TODOVIS - only moved part of the way */
		/* set position */
		robot_pos( r, &x, &y );
		r->r_x=(uint32_t)x;
		r->r_y=(uint32_t)y;
	}
}

void robot_damage_hit_robot( ROBOT *r, ROBOT *o )
{
	/* TODOVIS - Robots r and o collided  */
	/* LOG something for vis stuff */
	robot_damage( r );
	robot_damage( o );
}

void robot_damage_explosion_missile( ROBOT *r, MISSILE *m )
{
	/* TODOVIS - Robot r damaged by missile m */
	/* LOG something for vis stuff */
	robot_damage( r );
}

void robot_move( ROBOT *r, int32_t dist, ROBOT *robots, uint32_t nosrobs )
{
	double x,y,nx,ny;
	double ang_rad,ddist;
	uint32_t hit;
	int32_t ix,iy;
	uint32_t loop;
	/* Get current position */
	robot_pos( r, &x, &y );
	ang_rad=(double)r->r_head;
	ang_rad/=360.0;
	ang_rad*=(2.0*3.14159265);

	/* Max distance moveable in one turn */

	if( dist > 2 ) {
		dist=2;
	} else if( dist < 2 ) {
		dist=-2;
	}

	/* What if we're almost there ... */
	if( abs( r->r_for_dist - r->r_for_ct ) < 2 ) {
		dist = r->r_for_dist - r->r_for_ct;
	}

	ddist=(double)dist;

	/* move robot r dist along it's current heading */
	nx=x+(sin(ang_rad)*ddist);
	ny=y+(cos(ang_rad)*ddist);

	hit=0;	/* have we hit something between x,y and nx,ny */
	/* check for wall collisions */
	/* TODO - extra walls in arena? */
	ix=(int32_t)nx;
	iy=(int32_t)ny;
	if( ix < 5 || ix >= (ARENA_X-5) || iy < 5 || iy >= (ARENA_Y-5) ) {
		/* Hit wall */
		hit=1;
		/* Work out where as need to place robot at point it hits the wall */
	}
	/* check for robot collisions */
	loop=0;
	while( loop < nosrobs ) {
		if( r != &robots[loop] ) {
			double d;
			double ox,oy;
			robot_pos( &robots[loop], &ox, &oy );
			d=hypot( nx-ox, ny-oy );
			if( d < 10.0 ) {
				/* hit robot! damage both */
				hit=1;
				robot_damage_hit_robot( r, &robots[loop] );
			}
		}
		loop++;
	}

	/* missiles are checked by missile moving code */

	if( !hit ) {
		/* TODOVIS - moved a chunk without hitting anything */
		r->r_for_ct+=dist;
		if( abs( r->r_for_ct ) > abs( r->r_for_dist ) ) {
			fprintf( stderr, "SBR: r=%u robot_move for_ct > for_dist %d > %d\n", r->r_id, r->r_for_ct, r->r_for_dist ); 
			abort();
		}
		if( abs( r->r_for_ct ) == abs( r->r_for_dist ) ) {
			/* We're done */
			r->r_x=(uint32_t)x;
			r->r_y=(uint32_t)y;
			r->r_for_dist=0;
			r->r_for_ct=0;
			r->r_regi[REG_PC]+=4;
		}
		return;
	}
}

void missile_pos( MISSILE *m, double *x, double *y, double *sx, double *sy )
/*
 * Calculate the position of the tip of the missile
 */
{
	*x=-10.0;
	*y=-10.0;
	*sx=-10.0;
	*sy=-10.0;
	if( m->m_state == MISSILE_DEAD ) {
		fprintf( stderr, "ERROR: missile_pos called on dead missile\n" );
		return;
	}
	if( m->m_state == MISSILE_RUN ) {
		double mx,my,mr,mang;
		mx=(double)m->m_lx;
		my=(double)m->m_ly;
		mr=(double)(10+(2*m->m_ct));	/* 10 is the radius of the robot plus the barrel length */
		mang=(double)m->m_lang;
		mang/=360.0;
		mang*=(2.0*3.14159265);		/* radians */
		mx+=sin(mang)*mr;
		my+=cos(mang)*mr;
		*x=mx;
		*y=my;
		mx-=sin(mang)*5;		/* Start of missile co-ordinates */
		my-=cos(mang)*5;	
		*sx=mx;
		*sy=my;
		return;
	}
	if( m->m_state == MISSILE_EXPLOSION ) {
		*x=m->m_ex;
		*y=m->m_ey;
		return;
	}
	fprintf( stderr, "Unexpected missile state %u in missile_pos\n", m->m_state );
	return;
}

void cpu_setflagsf( ROBOT *r, float val )
{
	r->r_flags=0;
	if( val > 0.0 ) {
		r->r_flags |= IFLAG_GT;
	} else if( val < 0.0 ) {
		r->r_flags |= IFLAG_LT;
	} else {
		r->r_flags |= IFLAG_EQ;
	}
}

void cpu_setflags( ROBOT *r, int32_t val )
{
	r->r_flags=0;
	if( val > 0 ) {
		r->r_flags |= IFLAG_GT;
	} else if( val < 0 ) {
		r->r_flags |= IFLAG_LT;
	} else if( val == 0 ) {
		r->r_flags |= IFLAG_EQ;
	}
}

void do_scan( ROBOT *r, MISSILE *mi, double x, double y, int32_t bearing, ROBOT *robots, uint32_t nosrobs, uint32_t *ret_objtype, double *ret_ddistance, ROBOT **ret_rob, MISSILE **ret_mis, uint32_t pc_ct )
/*
 * Do the actual scanning
 */
{
	uint32_t objtype;
	double ddistance=10000.0;
	double d;
	uint32_t loop;
	MISSILE *m;
	objtype=0;
	ddistance=10000.0;
	*ret_objtype=0;
	*ret_ddistance=10000.0;
	*ret_rob=NULL;
	*ret_mis=NULL;

	/* check walls (there's always one) */
	/* LH wall */
	if( pf ) printf( "DEBUG: SCAN r=%u from %f,%f bearing %d\n", (r?r->r_id:999), x, y, bearing );
	d=dist_intersection_lines( x, y, bearing, 0, 0, 0 );
	if( d >= 0.0 ) {
		if( pf ) printf( "DEBUG: SCAN: Found LH wall at distance %f bearing %d\n", d, bearing );
		if( d < ddistance ) {
			if( pf ) printf( "DEBUG: SCAN: New closest wall on LH at distance=%f\n", d );
			ddistance=d;
		}
	}
	d=dist_intersection_lines( x, y, bearing, 0, 0, 90 );
	if( d >= 0.0 ) {
		if( pf ) printf( "DEBUG: SCAN: Found bottom wall at distance %f bearing %d\n", d, bearing );
		if( d < ddistance ) {
			if( pf ) printf( "DEBUG: SCAN: New closest wall on bottom at distance=%f\n", d );
			ddistance=d;
		}
	}
	d=dist_intersection_lines( x, y, bearing, ARENA_X, 0, 0 );
	if( d >= 0.0 ) {
		if( pf ) printf( "DEBUG: SCAN: Found RH wall at distance %f bearing %d\n", d, bearing );
		if( d < ddistance ) {
			if( pf ) printf( "DEBUG: SCAN: New closest wall on RH at distance=%f\n", d );
			ddistance=d;
		}
	}
	d=dist_intersection_lines( x, y, bearing, 0, ARENA_Y, 90 );
	if( d >= 0.0 ) {
		if( pf ) printf( "DEBUG: SCAN: Found top wall at distance %f bearing %d\n", d, bearing );
		if( d < ddistance ) {
			if( pf ) printf( "DEBUG: SCAN: New closest wall on top at distance=%f\n", d );
			ddistance=d;
		}
	}
	if( pf ) printf( "DEBUG: SCAN: Nearest wall is at %f\n", ddistance );
	/* check missiles */
	m=missiles;
	while( m ) {
		double mx, my, msx, msy;
		if( m != mi ) {
			/* If scanning as a missile, don't check itself! */
			if( m->m_state == MISSILE_RUN ) {
				missile_pos( m, &mx, &my, &msx, &msy );	/* Get current coordinates of missile*/
				d=dist_intersection_lines( msx, msy, m->m_lang, x, y, bearing );
				if( d >= 0 && d <= 5.0 ) {
					/* Radar line intersects with missile line close enough to missile */
					d=dist_intersection_lines( x, y, bearing, msx, msy, m->m_lang );
					if( pf ) printf( "DEBUG: SCAN: Found missile m=%p to at distance %f bearing %d\n", m, d, bearing );
					if( r == m->m_robot ) {
						if( pf ) printf( "DEBUG: SCAN: missile belongs to me!\n" );
					} else {
						if( pf ) printf( "DEBUG: SCAN: missile belongs to r=%u\n", m->m_robot->r_id );
					}
					if( d >= 0 && d < ddistance ) {
						if( pf ) printf( "DEBUG: SCAN: New closest is missile at distance=%f\n", d );
						*ret_mis=m;
						ddistance=d;
						objtype=RADAR_MISSILE;
					}
				}
			} else if( m->m_state == MISSILE_EXPLOSION ) {
				uint32_t mrad=0;
				uint32_t ct;
				missile_pos( m, &mx, &my, &msx, &msy );	/* Get current coordinates of missile*/
				ct=m->m_ct;
				if( ct <= 6 ) {
					mrad=(1+ct+ct);
				} else if( ct <= 12 ) {
					mrad=1+(12-ct)+(12-ct);
				}
				d=dist_intersection_line_circle( x, y, bearing, 1000, msx, msy, (double)mrad );
				if( d >= 0 ) {
					if( pf ) printf( "DEBUG: SCAN: Found explosion of m=%p at distance %f bearing %d\n", m, d, bearing );
					if( d >= 0 && d < ddistance ) {
						if( pf ) printf( "DEBUG: SCAN: New closest is missile explosion at distance=%f\n", d );
						*ret_mis=NULL;	/* clear if required */
						ddistance=d;
						objtype=RADAR_BLAST;
					}
				}
			}
		}
		m=m->m_next;
	}
	/* check robots */
	loop=0;
	while( loop < nosrobs ) {
		if( r != &robots[loop] ) {
			if( mi == NULL || robots[loop].r_missile != mi ) {
				/* Don't check the robot that filed the missile! */
				double ox,oy;
				robot_pos( &robots[loop], &ox, &oy );
				d=dist_intersection_line_circle( x, y, bearing, 10000, ox, oy, 5.0);
				if( d >= 0.0 ) {
					if( pf ) printf( "DEBUG: SCAN: Found other robot r=%u at distance %f\n", robots[loop].r_id, d );
					if( d < ddistance ) {
						ddistance=d;
						objtype=RADAR_ROBOT;
						if( pf ) printf( "DEBUG: SCAN: robot r=%u at distance %f is new closest\n", robots[loop].r_id, d );
						*ret_rob=&robots[loop];
						*ret_mis=NULL;	/* clear if required */
					}
				}
			}
		}
		loop++;
	}
	if( pf ) printf( "DEBUG: SCAN: robot r=%u object %u at distance %f %d \n", (r?r->r_id:999), objtype, ddistance, (int32_t)ddistance );
	*ret_objtype=objtype;
	*ret_ddistance=ddistance;
}

void cpu_laser( ROBOT *r, int32_t bearing, ROBOT *robots, uint32_t nosrobs, uint32_t pc_ct )
{
	double ddistance;
	uint32_t objtype;
	ROBOT *dummy_rob=NULL;
	MISSILE *mis=NULL;

	if( pf ) printf( "DEBUG: LASER r=%u from %u,%u bearing %d\n", r->r_id, r->r_x, r->r_y, bearing );

	do_scan( r, NULL, (double)r->r_x, (double)r->r_y, bearing, robots, nosrobs, &objtype, &ddistance, &dummy_rob, &mis, pc_ct );

	if( objtype == RADAR_MISSILE ) {
		/* Woohoo! */
		if( pf ) printf( "DEBUG: LASER r=%u hit a missile!\n", r->r_id );
		if( mis->m_robot == r ) {
			if( pf ) printf( "DEBUG: r=%u my own missile!\n", r->r_id );
		} else {
			if( pf ) printf( "DEBUG: r=%u someone elses missile!\n", r->r_id );
		}
		missile_laser( r, mis, pc_ct );
		/* TODOVIS - result of this */
	} else {
		if( pf ) printf( "DEBUG: LASER r=%u did nothing as hit a %u\n", r->r_id, objtype );
		/* TODOVIS - result of this */
	}
}

void cpu_radar( ROBOT *r, uint32_t r_b, uint32_t r_o, uint32_t r_d, ROBOT *robots, uint32_t nosrobs, uint32_t pc_ct )
{
	double ddistance;
	uint32_t objtype;
	int32_t bearing;
	ROBOT *dummy_rob=NULL;
	MISSILE *dummy_mis=NULL;
	/* perform RADAR scan along bearing defined by register r_b */
	/* Put object found in register r_o */
	/* Put distance to object in register r_d */

	if( r_b < REG_FLT_BASE ) {
		/* Integer bearing */
		bearing = r->r_regi[r_b];
	} else {
		bearing = (int32_t)(r->r_regf[r_b-REG_FLT_BASE]);
	}

	if( pf ) printf( "DEBUG: RADAR r=%u from %u,%u bearing %d\n", r->r_id, r->r_x, r->r_y, bearing );

	do_scan( r, NULL, (double)r->r_x, (double)r->r_y, bearing, robots, nosrobs, &objtype, &ddistance, &dummy_rob, &dummy_mis, pc_ct );

	/* TODOVIS - result of this */
	if( pf ) printf( "DEBUG: RADAR: robot r=%u object %u at distance %f %d \n", r->r_id, objtype, ddistance, (int32_t)ddistance );
	r->r_regi[r_o]=objtype;
	r->r_regi[r_d]=(int32_t)ddistance;
}

uint32_t cpu_exec( ROBOT *r, ROBOT *robots, uint32_t nosrobs, uint32_t pc_ct, uint32_t seed )
{
	INSTRUCTION i;
	uint32_t *ival;
	int32_t r_pc;
	uint32_t iflag_s=0;

	/* Check if mid instruction - moving/turning */
	if( r->r_rot_ct != 0 ) {
		if( abs( r->r_rot_ct ) <= 2 ) {
			/* Finished this instruction */
			r->r_regi[REG_PC]+=4; /* TODOVIS - turned r->r_rot_ct */
			r->r_head += r->r_rot_ct;
			if( r->r_head >= 360 ) { r->r_head%=360; } else if( r->r_head < 0 ) { r->r_head+=360; }
			r->r_rot_ct=0;
			return( 1 );
		}
		/* Still got lots to turn, crank out another 2 deg */
		if( r->r_rot_ct > 0 ) {
			r->r_head += 2;
			r->r_head %= 360;
			/* TODOVIS - ROT 2 */
			r->r_rot_ct -= 2;
		} else {
			r->r_head -= 2;
			if( r->r_head < 0 ) r->r_head+=360;
			/* TODOVIS - ROT -2 */
			r->r_rot_ct += 2;
		}
		/* IFLAG_S means nothing on ROT */
		return(1);
	}
	if( r->r_for_ct != 0 ) {
		int32_t dist;
		uint32_t x;
		dist = r->r_for_dist - r->r_for_ct;	/* how far to go */
		if( dist > 0 ) {
			robot_move( r, 2, robots, nosrobs );
		} else {
			robot_move( r, -2, robots, nosrobs );
		}
		/* PC increment is handled in robot_move */
		return( 1 );
		/* IFLAG_S means nothing on FOR */
	}

	/* Get instruction, decode it, execute it */
	bzero( &i, sizeof( INSTRUCTION ) );

	r_pc = r->r_regi[REG_PC];
	if( r_pc < 0 || r_pc >= MAX_MEMORY ) {
		fprintf( stderr, "robot invalid pc=%d\n", r_pc );
		/* TODOVIS - MEMFAULT */
		r->r_state=STATE_MEMFAULT;
		return( 1 );
	}
	ival=(uint32_t *)&(r->r_memory[r_pc]);
	i.i_image = *ival;
	(void)instr_decode( &i );	/* TODO - return val? */
	if( pf ) printf( "INSTR: r=%u pc_ct=%u r_pc=%d %8x opcode=%hu(%s) flags=%x rflags=%x amode=%hu\n", r->r_id, pc_ct, r_pc, i.i_image, i.i_opcode, opcode(i.i_opcode), i.i_flags, r->r_flags, i.i_amode );
	/* Check instruction flags, do we even need to execute it? */
	if( i.i_flags & IFLAG_S ) {
		iflag_s=1;
	}
	if( ( ( i.i_flags & 7 ) & ( r->r_flags & 7 ) ) != ( i.i_flags & 7 ) ) {
		/* Flags do not match, do not execute */
		if( pf ) printf( "DEBUG: no exec iflags=%2x rflags=%2x\n", i.i_flags, r->r_flags );
		r->r_regi[REG_PC]+=4;	/* TODOVIS - not exec */
		/* TODO - what do we do with the r_flags here if iflag_s is set? */
		return( 0 );
	}
	/* We're executing it */
		
	switch( i.i_opcode ) {
		case OPCODE_STP:
			r->r_regi[REG_PC]+=4;	/* TODOVIS - STP */
			robot_stop( r );
			/* IFLAG_S doesn't matter */
			return( 1 );
			break;
		case OPCODE_NOP:
			r->r_regi[REG_PC]+=4;	/* TODOVIS - NOP */
			if( iflag_s ) { r->r_flags=0; }	/* clear flags on a NOPS */
			return( 1 );
			break;
		case OPCODE_FOR:
			{
				int32_t val;
				uint32_t regno;
				
				regno=i.i_op1;
				if( i.i_amode != IAMODE_DIRECT ) {
					fprintf( stderr, "ERROR: robot r=%u unknown amode %u for opcode %hu ABENDING\n", r->r_id, i.i_amode, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				if( regno < 0 || regno >= REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for opcode %hu ABENDING\n", r->r_id, regno, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}

				val=r->r_regi[regno];

				if( val == 0 ) {
					/* Asked to move 0 */
					r->r_regi[REG_PC]+=4;	/* TODOVIS - effective NOP */
					return( 1 );
					break;
				}
				r->r_for_dist=val;
				r->r_for_ct=0;
				if( pf ) printf( "ARGLE: FOR=%d\n", val );
				robot_move( r, r->r_for_dist, robots, nosrobs );
				/* PC increment handle by robot_move */
				/* IFLAG_S not relevant */

				return( 1 );
				break;
			}
		case OPCODE_ROT:
			{
				int32_t val;
				uint32_t regno;
				
				regno=i.i_op1;
				if( i.i_amode != IAMODE_DIRECT ) {
					fprintf( stderr, "ERROR: robot r=%u unknown amode %u for opcode %hu ABENDING\n", r->r_id, i.i_amode, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				if( regno < 0 || regno >= REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for opcode %hu ABENDING\n", r->r_id, regno, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}

				val=r->r_regi[regno];

				if( val == 0 ) {
					/* Asked to turn 0 degrees */
					r->r_regi[REG_PC]+=4;	/* TODOVIS - effective NOP */
					return( 1 );
					break;
				} else if( abs(val) <= 2 ) {
					/* Asked to turn 1 or 2 degrees, can be completed in a turn */
					r->r_head += val;
					if( r->r_head >= 360 ) { r->r_head%=360; } else if( r->r_head < 0 ) { r->r_head+=360; }
					r->r_regi[REG_PC]+=4;	/* TODOVIS - ROT n */
					return( 1 );
					break;
				}
				/* Here because we're greater than 2 degrees */

				if( val > 0 ) {
					r->r_head += 2;
					r->r_rot_ct = val-2;
				} else {
					r->r_head -= 2;
					r->r_rot_ct = val+2;
				}
				if( r->r_head >= 360 ) { r->r_head%=360; } else if( r->r_head < 0 ) { r->r_head+=360; }
				/* Keep PC on this instruction */

				return( 1 );
				break;
			}
		case OPCODE_CMP:
			{
				int32_t val=0;
				uint32_t regno=0;
				uint32_t regno2=0;

				r->r_regi[REG_PC]+=4;

				regno=i.i_op1;
				if( i.i_amode == IAMODE_ABSOLUTE ) {
					val=i.i_op2;
				} else if( i.i_amode == IAMODE_DIRECT ) {
					regno2=i.i_op2;
					if( regno < 0 || regno > 127 ) {
						fprintf( stderr, "ERROR: robot r=%u invalid regno %u for opcode %hu ABENDING\n", r->r_id, regno, i.i_opcode );
						robot_abend( r );
						return( 1 );
						break;
					}
				} else {
					fprintf( stderr, "ERROR: robot r=%u unknown amode %u for opcode %hu ABENDING\n", r->r_id, i.i_amode, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}

				if( regno < 0 || regno > 127 ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for opcode %hu ABENDING\n", r->r_id, regno, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}

				if( i.i_amode == IAMODE_ABSOLUTE ) {
					if( regno < REG_FLT_BASE ) {
						/* Integer comparison */
						if( r->r_regi[regno] == val ) {
							r->r_flags=IFLAG_EQ;
						} else if( r->r_regi[regno] > val ) {
							/* TODO - is this the right way round? */
							r->r_flags=IFLAG_GT;
						} else {
							r->r_flags=IFLAG_LT;
						}
					} else {
						float fval=(float)val;
						regno-=REG_FLT_BASE;
						if( fabs( fval - r->r_regf[regno] ) < 0.00001 ) {
							r->r_flags=IFLAG_EQ;
						} else if( r->r_regf[regno] > fval ) {
							r->r_flags=IFLAG_GT;
						} else {
							r->r_flags=IFLAG_LT;
						}
							
					}
				} else if( i.i_amode == IAMODE_DIRECT ) {
					if( regno < REG_FLT_BASE && regno2 < REG_FLT_BASE ) {
						if( r->r_regi[regno] == r->r_regi[regno2] ) {
							r->r_flags=IFLAG_EQ;
						} else if( r->r_regi[regno] > r->r_regi[regno2] ) {
							r->r_flags=IFLAG_GT;
						} else {
							r->r_flags=IFLAG_LT;
						}
					} else if( regno >= REG_FLT_BASE && regno2 >= REG_FLT_BASE ) {
						if( r->r_regi[regno] > r->r_regi[regno2] ) {
							r->r_flags=IFLAG_GT;
						} else if( r->r_regi[regno] < r->r_regi[regno2] ) {
							r->r_flags=IFLAG_LT;
						} else {
							r->r_flags=IFLAG_EQ;
						}
					} else {
						/* TODO - mixed CMP */
						fprintf( stderr, "ERROR: robot r=%u diff reg types %u %u for opcode %hu ABENDING\n", r->r_id, regno, regno2, i.i_opcode );
						robot_abend( r );
						return( 1 );
						break;
					}
				}
				/* IFLAG_S is implicit! */
			}
			return( 0 );
			break;
		case OPCODE_LSR:
			{
				uint32_t regno_bearing;
				int32_t bearing;

				r->r_regi[REG_PC]+=4;

				regno_bearing=i.i_op1;
				if( regno_bearing < 0 || regno_bearing > REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for bearing for opcode %hu ABENDING\n", r->r_id, regno_bearing, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				bearing=r->r_regi[regno_bearing];
				cpu_laser( r, bearing, robots, nosrobs, pc_ct );
				/* IFLAG_S has no meaning */
				/* TODOVIS */
			}
			return(1);
			break;
		case OPCODE_LNC:
			{
				uint32_t regno_range;
				int32_t range;

				r->r_regi[REG_PC]+=4;

				regno_range=i.i_op1;
				if( regno_range < 0 || regno_range > 127 ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for range for opcode %hu ABENDING\n", r->r_id, regno_range, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				if( regno_range < REG_FLT_BASE ) {
					range=r->r_regi[regno_range];
				} else {
					range=(int32_t)r->r_regf[regno_range-REG_FLT_BASE];
				}
				missile_new( r, range, pc_ct );
				/* TODOVIS */
			}
			return(1);
			break;
		case OPCODE_SHD:
			{
				uint32_t regno;

				r->r_regi[REG_PC]+=4;

				regno=i.i_op1;
				if( regno < 0 || regno >= REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for opcode %hu ABENDING\n", r->r_id, regno, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				if( r->r_regi[regno] == 0 ) {
					if( r->r_shield ) {
						/* TODOVIS - shield off */
						r->r_shield=0;
						return(1);
						break;
					}
				} else {
					if( r->r_shield == 0 ) {
						if( r->r_fuel > 0 ) {
							/* TODOVIS - shield on */
							r->r_shield=1;
							return(1);
							break;
						} else {
							if( pf ) printf( "DEBUG: robot r=%u has no fuel for shield\n", r->r_id );
						}
					}
				}
				/* IFLAG_S does nothing here */
			}
			return(0);
			break;
		case OPCODE_AND:
		case OPCODE_OR:
		case OPCODE_XOR:
			{
				uint32_t regno_a;
				uint32_t regno_b;
				uint32_t regno_c;
				int32_t res;

				r->r_regi[REG_PC]+=4;

				regno_a=i.i_op1;
				if( regno_a < 0 || regno_a >= REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for a for opcode %hu ABENDING\n", r->r_id, regno_a, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				regno_b=i.i_op2;
				if( regno_b < 0 || regno_b >= REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for b for opcode %hu ABENDING\n", r->r_id, regno_b, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				regno_c=i.i_op3;
				if( regno_c < 0 || regno_c >= REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for c for opcode %hu ABENDING\n", r->r_id, regno_c, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				if( i.i_opcode == OPCODE_AND ) {
					res=(r->r_regi[regno_b]) & (r->r_regi[regno_c]);
				} else if( i.i_opcode == OPCODE_OR ) {
					res=(r->r_regi[regno_b]) | (r->r_regi[regno_c]);
				} else if( i.i_opcode == OPCODE_XOR ) {
					res=(r->r_regi[regno_b]) ^ (r->r_regi[regno_c]);
				}
				r->r_regi[regno_a]=res;
				if( iflag_s ) {
					/* Set flags based on result */
					cpu_setflags( r, res );
				}
			}
			return(0);
			break;
		case OPCODE_SUB:
		case OPCODE_DIV:
		case OPCODE_ADD:
		case OPCODE_MUL:
			{
				uint32_t regno_a;
				uint32_t regno_b;
				uint32_t regno_c;
				int32_t res;

				r->r_regi[REG_PC]+=4;

				regno_a=i.i_op1;
				if( regno_a < 0 || regno_a >= REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for a for opcode %hu ABENDING\n", r->r_id, regno_a, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				regno_b=i.i_op2;
				if( regno_b < 0 || regno_b >= REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for b for opcode %hu ABENDING\n", r->r_id, regno_b, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				regno_c=i.i_op3;
				if( regno_c < 0 || regno_c >= REG_FLT_BASE ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for c for opcode %hu ABENDING\n", r->r_id, regno_c, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				if( i.i_opcode == OPCODE_SUB ) {
					res=(r->r_regi[regno_b])-(r->r_regi[regno_c]);
					/* TODO - UNDERFLOW? */
				} else if( i.i_opcode == OPCODE_MUL) {
					res=(r->r_regi[regno_b])*(r->r_regi[regno_c]);
					/* TODO - OVERFLOW? */
				} else if( i.i_opcode == OPCODE_ADD) {
					res=(r->r_regi[regno_b])+(r->r_regi[regno_c]);
					/* TODO - OVERFLOW? */
				} else if( i.i_opcode == OPCODE_DIV ) {
					if( r->r_regi[regno_c] == 0 ) {
						fprintf( stderr, "ERROR: robot %u div by 0 for opcode %hu ABENDING\n", r->r_id, i.i_opcode );
						robot_abend( r );
						return( 1 );
						break;
					}
					res=(r->r_regi[regno_b])/(r->r_regi[regno_c]);
				}
				r->r_regi[regno_a]=res;
				if( iflag_s ) {
					/* Set flags based on result */
					cpu_setflags( r, res );
				}
			}
			return(0);
			break;
		case OPCODE_RDR:
			{
				uint32_t regno_bearing;
				uint32_t regno_obj;
				uint32_t regno_dist;
				int32_t bearing_i;
				double bearing_f;

				r->r_regi[REG_PC]+=4;

				regno_bearing=i.i_op1;
				if( regno_bearing < 0 || regno_bearing > 127 ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for bearing for opcode %hu ABENDING\n", r->r_id, regno_bearing, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				regno_obj=i.i_op2;
				if( regno_obj < 0 || regno_obj > 63 ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for obj for opcode %hu ABENDING\n", r->r_id, regno_obj, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				regno_dist=i.i_op3;
				if( regno_dist < 0 || regno_dist > 127 ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for dist for opcode %hu ABENDING\n", r->r_id, regno_dist, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}

				cpu_radar( r, regno_bearing, regno_obj, regno_dist, robots, nosrobs, pc_ct );

				if( iflag_s ) {
					/* Set flags based on contents of object register */
					cpu_setflags( r, r->r_regi[regno_obj] );
				}
			}
			return(1);
			break;
		case OPCODE_HED:
		case OPCODE_DAM:
		case OPCODE_TIC:
		case OPCODE_MIS:
		case OPCODE_NOS:
		case OPCODE_WAL:
			{
				uint32_t regno=0;
				int32_t val;

				r->r_regi[REG_PC]+=4;

				switch( i.i_opcode ) {
					case OPCODE_HED: val=r->r_head; break;
					case OPCODE_TIC: val=pc_ct; break;
					case OPCODE_DAM: val=r->r_health; break;
					case OPCODE_MIS: val=(r->r_missile?1:0); break;
					case OPCODE_NOS: 
						{
							uint32_t loop;
							val=0;
							loop=0;
							while( loop < nosrobs ) {
								if( robots[loop].r_state == STATE_RUN ) {
									val++;
								}
								loop++;
							}
						}
						break; /* nos robots */
					case OPCODE_WAL: val=0; break;	/* No walls */
					default:
						fprintf( stderr, "ERROR: robot r=%u unhandled sub-opcode %hu for opcode %hu ABENDING\n", r->r_id, i.i_opcode, i.i_opcode );
						robot_abend( r );
						return( 1 );
						break;
				}

				if( i.i_amode != IAMODE_DIRECT ) {
					fprintf( stderr, "ERROR: robot r=%u unknown amode %u for opcode %hu ABENDING\n", r->r_id, i.i_amode, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				regno=i.i_op1;
				if( regno < 0 || regno > 127 ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for opcode %hu ABENDING\n", r->r_id, regno, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				if( regno < REG_FLT_BASE ) {
					r->r_regi[regno]=val;
				} else {
					r->r_regf[regno-REG_FLT_BASE]=(float)val;
				}
				if( iflag_s ) {
					cpu_setflags( r, val );
				}
			}
			return(0);
			break;
		case OPCODE_LDR:
			{
				int32_t val=0;
				float fval=0.0;
				uint32_t regno=0,regno2;

				r->r_regi[REG_PC]+=4;
				regno=i.i_op1;
				if( i.i_amode == IAMODE_ABSOLUTE ) {
					val=i.i_op2;
				} else if( i.i_amode == IAMODE_DIRECT ) {
					regno2=i.i_op2;
					if( regno < 0 || regno > 127 ) {
						fprintf( stderr, "ERROR: robot r=%u invalid regno2 %u for opcode %hu ABENDING\n", r->r_id, regno, i.i_opcode );
						robot_abend( r );
						return( 1 );
						break;
					}
				} else {
					fprintf( stderr, "ERROR: robot r=%u unknown amode %u for opcode %hu ABENDING\n", r->r_id, i.i_amode, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				if( regno < 0 || regno > 127 ) {
					fprintf( stderr, "ERROR: robot r=%u invalid regno %u for opcode %hu ABENDING\n", r->r_id, regno, i.i_opcode );
					robot_abend( r );
					return( 1 );
					break;
				}
				if( pf ) printf( "DEBUG: robot r=%u setting reg %u to %d\n", r->r_id, regno, val );
				if( regno < REG_FLT_BASE ) {
					if( i.i_amode == IAMODE_ABSOLUTE ) {
						;
					} else if( i.i_amode == IAMODE_DIRECT ) {
						if( regno2 < REG_FLT_BASE ) {
							val=r->r_regi[regno2];
						} else {
							val=(int32_t)r->r_regf[regno2-REG_FLT_BASE];
						}
					}
					r->r_regi[regno]=val;
					if( iflag_s ) {
						cpu_setflags( r, val );
					}
				} else {
					if( i.i_amode == IAMODE_ABSOLUTE ) {
						fval=(float)val;
					} else if( i.i_amode == IAMODE_DIRECT ) {
						if( regno2 < REG_FLT_BASE ) {
							fval=(float)r->r_regi[regno2];
						} else {
							fval=r->r_regf[regno2-REG_FLT_BASE];
						}
					}
					r->r_regf[regno-REG_FLT_BASE]=fval;
					if( iflag_s ) {
						cpu_setflagsf( r, fval );
					}
				}
			}
			return(0);
			break;
		case OPCODE_BLI:
		case OPCODE_JMP:
			if( i.i_opcode == OPCODE_BLI ) {
				r->r_regi[REG_LR]=r->r_regi[REG_PC]+4;	/* For BLI set link register to next instruction */
			}
			/* find address we're branching to */
			if( i.i_amode == IAMODE_MEMADDR ) {
				int32_t addr;
				addr=(i.i_op1<<9)|(i.i_op2<<2);
				if( pf ) printf( "DEBUG: BLI to absolute address %u\n", addr );
				if( addr < 0 || addr >= MAX_MEMORY ) {
					fprintf( stderr, "robot invalid pc=%d\n", addr );
					/* TODOVIS - MEMFAULT */
					r->r_state=STATE_MEMFAULT;
					return(1);
				}
				r->r_regi[REG_PC]=addr;
			} else {
				fprintf( stderr, "ERROR: robot r=%u unknown amode %u for opcode %hu ABENDING\n", r->r_id, i.i_amode, i.i_opcode );
				robot_abend( r );
				return(1);
				break;
			}
			/* IFLAG_S not relevant? */
			return( 0 );
			break;
		default:
			fprintf( stderr, "ERROR: robot r=%u unknown opcode %hu ABENDING seed=%u\n", r->r_id, i.i_opcode, seed );
			robot_abend( r );
			return(1);
			break;
	}
	fprintf( stderr, "SBR: cpu_exec\n" );
	abort();
	return(1);
}

void arena_exec( uint32_t seed, uint32_t nosrobs, ROBOT *robots )	/* wall options, arena size? */
{
	uint32_t loop;
	uint32_t alive;
	uint32_t pc_ct;
	prng_init( seed );

	loop=0;
	while( loop < nosrobs ) {
		ROBOT *r=&robots[loop];
		bzero( robots[loop].r_regi, sizeof( uint32_t ) * REG_NOS_MAX );
		bzero( robots[loop].r_regf, sizeof( float ) * REG_NOS_MAX );
		robots[loop].r_id=loop;
		robots[loop].r_state=STATE_RUN;
		robots[loop].r_fuel=MAX_FUEL;
		robots[loop].r_health=MAX_HEALTH;
		robots[loop].r_damage_ct=0;
		robots[loop].r_for_ct=0;
		robots[loop].r_rot_ct=0;
		robots[loop].r_shield=SHIELD_OFF;
		robots[loop].r_flags=0;
		do { 
			robots[loop].r_x=5+prng_rand( NULL, ARENA_X-10 );
			robots[loop].r_y=5+prng_rand( NULL, ARENA_Y-10 );
			/* TODOWALL - Check with internal wall overlap too */
		} while( arena_overlapcheck( robots[loop].r_x, robots[loop].r_y, loop, robots ) == ARENA_ERR );
		robots[loop].r_head=prng_rand( NULL, 360 );
		if( pf_vis ) printf( "RSTATE: r=%u %u pos=%u,%u h=%u damage_ct=%u rot_ct=%d for_ct=%d health=%u fuel=%u shield=%u\n", r->r_id, pc_ct, r->r_x, r->r_y, r->r_head, r->r_damage_ct, r->r_rot_ct, r->r_for_ct, r->r_health, r->r_fuel, r->r_shield );
		loop++;
	}

	alive=nosrobs;
	pc_ct=0;
	while( alive > 1 && pc_ct < 10000 ) {
		MISSILE *m;
		/* Check fuel levels for all robots with shields on */
		if( pf ) printf( "\n" );
		alive=0;
		loop=0;
		while( loop < nosrobs ) {
			ROBOT *r=&robots[loop];
			if( r->r_state == STATE_RUN ) {
				alive++;
				if( r->r_shield == SHIELD_ON ) {
					r->r_fuel--;
					if( r->r_fuel == 0 ) {
						/* TODOVIS - SHIELD OFF EVENT FOR REPLAY */
						/* TODOVIS - OUT OF FUEL EVENT FOR REPLAY */
						r->r_shield=SHIELD_OFF;
					}
				}
			}
			loop++;
		}
		/* MISSILES/EXPLOSIONS */
		m=missiles;
		while( m ) {
			if( m->m_state == MISSILE_RUN ) {
				double mx,my,msx,msy;
				uint32_t mang=m->m_lang;
				m->m_ct++;
				missile_pos( m, &mx, &my, &msx, &msy );	/* Get current coordinates */
				if( m->m_ct >= m->m_range ) {
					/* BOOM */
					if( pf ) printf( "DEBUG: missile explode at %f,%f\n", mx, my );
					m->m_state=MISSILE_EXPLOSION;
					m->m_ex=mx;			/* Set explosion coords */
					m->m_ey=my;
					m->m_ct=0;
					/* Explosions affecting robots will be checked in the next section as we fall through */
				} else {
					double ddistance;
					uint32_t objtype;
					ROBOT *dummy_rob=NULL;
					MISSILE *dummy_mis=NULL;
					/* Check if the missile has hit anything early */
					/* do a scan */
					do_scan( NULL, m, msx, msy, m->m_lang, robots, nosrobs, &objtype, &ddistance, &dummy_rob, &dummy_mis, pc_ct );
					if( ddistance < 2.0 ) {
						/* Hit something within 2... */
						if( pf ) printf( "DEBUG: missile hit early %f,%f to %f,%f objtype=%u, dist=%f\n", msx, msy, mx, my, objtype, ddistance );
						m->m_state=MISSILE_EXPLOSION;
						m->m_ex=mx;			/* Set explosion coords */
						m->m_ey=my;
						m->m_ct=0;
					}
				}
			}
			if( m->m_state == MISSILE_EXPLOSION ) {
				/* Missile is now exploding */
				/* We have a point of detonation and a blast radius (2*ct)+1 */
				/* Blast radius starts at 0 and increases to 12 and then reduces back to 0 then disappears and missile becomes dead */
				double mx,my,mrad;
				uint32_t ct;
				ct=m->m_ct;
				mx=(double)m->m_ex;
				my=(double)m->m_ey;
				if( ct <= 12 ) {
					if( ct <= 6 ) {
						mrad=(double)(1+ct+ct);
					} else if( ct <= 12 ) {
						mrad=(double)(1+(12-ct)+(12-ct));
					}
					if( pf ) printf( "DEBUG: mrad=%f\n", mrad );
					/* Find any robots that it could have hit? */
					loop=0;
					while( loop < nosrobs ) {
						ROBOT *r;
						r=&robots[loop];
						if( r->r_state == STATE_RUN ) {
							/* If robot is running and not already recovering from damage and doesn't have shields on */
							double rx,ry,d;
							robot_pos( r, &rx, &ry );
							d=hypot( rx-mx, ry-my );
							if( pf ) printf( "DEBUG: distance between explosion and robot r=%u is %f\n", r->r_id, d );
							if( d <= mrad ) {
								if( pf ) printf( "DEBUG: robot r=%u caught by missile explosion\n", r->r_id );
								if( r->r_damage_ct > 0 ) {
									if( pf ) printf( "DEBUG: robot r=%u not damaged by missile explosion because damage_ct\n", r->r_id );
								} else if( r->r_shield == SHIELD_ON ) {
									if( pf ) printf( "DEBUG: robot r=%u not damaged by missile explosion because shield\n", r->r_id );
								} else {
									if( pf ) printf( "DEBUG: robot r=%u damaged by missile explosion!\n", r->r_id );
									robot_damage_explosion_missile( r, m );
								}
							}
						}
						loop++;
					}
				} else {
					/* Missile done exploding */
					m->m_state = MISSILE_DEAD;
					/* Gets cleared up later but robot still can't fire this cpu cycle */
				}
				m->m_ct++;
			}
			m=m->m_next;
		}
		/* ROBOTS */
		if( pf_vis ) printf( "PSTATE: %u\n", pc_ct );
		loop=0;
		while( loop < nosrobs ) {
			ROBOT *r=&robots[loop];
			if( r->r_state == STATE_RUN ) {
				double rx,ry;
				robot_pos( r, &rx, &ry ); 
				if( pf_vis ) printf( "RSTATE: r=%u %u pos=%u,%u h=%u damage_ct=%u rot_ct=%d for_ct=%d health=%u fuel=%u shield=%u\n", r->r_id, pc_ct, (uint32_t)rx, (uint32_t)ry, r->r_head, r->r_damage_ct, r->r_rot_ct, r->r_for_ct, r->r_health, r->r_fuel, r->r_shield );
				if( r->r_damage_ct > 0 ) {
					/* If damaged then entire cpu cycle is used up */
					r->r_damage_ct--;
					if( r->r_damage_ct == 0 && r->r_health == 0 ) {
						robot_dead( r, pc_ct );
					} else {
						/* TODOVIS - log something about being damaged this iteration */
					}
				} else {
					/* normal processing */
					cpu_exec( r, robots, nosrobs, pc_ct, seed );
				}
			} else {
				if( pf_vis ) printf( "RSTATE: r=%u DEAD: %u\n", r->r_id, r->r_state );
			}
			loop++;
		}

		/* Clean up dead missiles */
		while( missiles && missiles->m_state == MISSILE_DEAD ) {
			missile_free( missiles );
			/* Keep cleaning off first missile in linked list if it is dead */
			missiles=missiles->m_next;
		}
		m=missiles;
		while( m ) {
			if( m->m_next && m->m_next->m_state == MISSILE_DEAD ) {
				missile_free( m->m_next );
				m->m_next = m->m_next->m_next;
			} else {
				m=m->m_next;
			}
		}
		m=missiles;
		while( m ) {
			if( m->m_state == MISSILE_RUN ) {
				double mx,my,msx,msy;
				missile_pos( m, &mx, &my, &msx, &msy );	/* Get current coordinates of missile*/
				if( pf_vis ) printf( "MSTATE: m=%p r=%u state=R pos=%u,%u ang=%u range=%u ct=%u\n", m, m->m_robot->r_id, (uint32_t)msx, (uint32_t)msy, m->m_lang, m->m_range, m->m_ct );
			} else if( m->m_state == MISSILE_EXPLOSION ) {
				if( pf_vis ) printf( "MSTATE: m=%p r=%u state=E pos=%u,%u ct=%u\n", m, m->m_robot->r_id, (uint32_t)m->m_ex, (uint32_t)m->m_ey, m->m_ct );
			}
			m=m->m_next;
		}
		/* Increment number of instructions processed */
		if( pf_vis ) printf( "PSTATE_END\n" );
		pc_ct++;
	}
	printf( "\n" );
	printf( "FINAL seed=%u\n", seed );
	loop=0;
	while( loop < nosrobs ) {
		ROBOT *r=&robots[loop];
		double rx,ry;
		if( r->r_state == STATE_RUN ) {
			r->r_pc=pc_ct;
		}
		robot_pos( r, &rx, &ry ); 
		printf( "RSTATE_FINAL: r=%u loop=%u cycles=%u pos=%u,%u h=%u damage_ct=%u rot_ct=%d for_ct=%d health=%u fuel=%u\n", r->r_id, loop, r->r_pc, (uint32_t)rx, (uint32_t)ry, r->r_head, r->r_damage_ct, r->r_rot_ct, r->r_for_ct, r->r_health, r->r_fuel );
		loop++;
	}
	
}
