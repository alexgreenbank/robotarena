#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "arena.h"
#include "hash.h"
#include "instr.h"

typedef struct OPCODE {
	char text[16];
	uint32_t opcode;
	uint32_t len;
} OPCODE;

OPCODE opcode_array[]={
	{ "BLI", OPCODE_BLI, 3 },
	{ "B", OPCODE_JMP, 1 },
	{ "JMP", OPCODE_JMP, 3 },
	{ "NOP", OPCODE_NOP, 3 },
	{ "MUL", OPCODE_MUL, 3 },
	{ "DIV", OPCODE_DIV, 3 },
	{ "SUB", OPCODE_SUB, 3 },
	{ "ADD", OPCODE_ADD, 3 },
	{ "AND", OPCODE_AND, 3 },
	{ "OR", OPCODE_OR, 2 },
	{ "XOR", OPCODE_XOR, 3 },
	{ "NOT", OPCODE_NOT, 3 },
	{ "ROL", OPCODE_ROL, 3 },
	{ "ROR", OPCODE_ROR, 3 },
	{ "SFL", OPCODE_SFL, 3 },
	{ "SFR", OPCODE_SFR, 3 },
	{ "SQR", OPCODE_SQR, 3 },
	{ "SIN", OPCODE_SIN, 3 },
	{ "COS", OPCODE_COS, 3 },
	{ "TAN", OPCODE_TAN, 3 },
	{ "ASIN", OPCODE_ASIN, 4 },
	{ "ACOS", OPCODE_ACOS, 4 },
	{ "ATAN", OPCODE_ATAN, 4 },
	{ "POS", OPCODE_POS, 3 },
	{ "RND", OPCODE_RND, 3 },
	{ "FOR", OPCODE_FOR, 3 },
	{ "REV", OPCODE_REV, 3 },
	{ "ROT", OPCODE_ROT, 3 },
	{ "LSR", OPCODE_LSR, 3 },
	{ "RDR", OPCODE_RDR, 3 },
	{ "LNC", OPCODE_LNC, 3 },
	{ "NOS", OPCODE_NOS, 3 },
	{ "SHD", OPCODE_SHD, 3 },
	{ "POW", OPCODE_POW, 3 },
	{ "DAM", OPCODE_DAM, 3 },
	{ "HED", OPCODE_HED, 3 },
	{ "STP", OPCODE_STP, 3 },
	{ "MIS", OPCODE_MIS, 3 },
	{ "TIC", OPCODE_TIC, 3 },
	{ "WAL", OPCODE_WAL, 3 },
	{ "PRR", OPCODE_PRR, 3 },
	{ "PRS", OPCODE_PRS, 3 },
	{ "PSH", OPCODE_PSH, 3 },
	{ "POP", OPCODE_POP, 3 },
	{ "CMP", OPCODE_CMP, 3 },
	{ "LDR", OPCODE_LDR, 3 },
	{ "MOV", OPCODE_LDR, 3 },
	{ "STR", OPCODE_STR, 3 },
	{ "Z", 0, 0 }
};

OPCODE flag_array[]={
	{ "EQ", IFLAG_EQ, 2 },
	{ "GTE", IFLAG_EQ|IFLAG_GT, 3 },
	{ "LTE", IFLAG_EQ|IFLAG_LT, 3 },
	{ "GT", IFLAG_GT, 2 },
	{ "LT", IFLAG_LT, 2 },
	{ "NE", IFLAG_GT|IFLAG_LT, 2 },
	{ "NV", IFLAG_GT|IFLAG_LT|IFLAG_EQ, 2 },
	{ "Z", 0, 0 }
};

char *opcode( uint16_t n )
{
	if( n > OPCODE_LIMIT_NOADDRMODE ) {
		n -= ((n-OPCODE_LIMIT_NOADDRMODE)%6);
	}
	switch( n ) {
		case OPCODE_BLI: return( "BLI" ); break;
		case OPCODE_JMP: return( "JMP" ); break;
		case OPCODE_NOP: return( "NOP" ); break;
		case OPCODE_MUL: return( "MUL" ); break;
		case OPCODE_DIV: return( "DIV" ); break;
		case OPCODE_SUB: return( "SUB" ); break;
		case OPCODE_ADD: return( "ADD" ); break;
		case OPCODE_AND: return( "AND" ); break;
		case OPCODE_OR: return( "OR" ); break;
		case OPCODE_XOR: return( "XOR" ); break;
		case OPCODE_NOT: return( "NOT" ); break;
		case OPCODE_ROL: return( "ROL" ); break;
		case OPCODE_ROR: return( "ROR" ); break;
		case OPCODE_SFL: return( "SFL" ); break;
		case OPCODE_SFR: return( "SFR" ); break;
		case OPCODE_SQR: return( "SQR" ); break;
		case OPCODE_SIN: return( "SIN" ); break;
		case OPCODE_COS: return( "COS" ); break;
		case OPCODE_TAN: return( "TAN" ); break;
		case OPCODE_ASIN: return( "ASIN" ); break;
		case OPCODE_ACOS: return( "ACOS" ); break;
		case OPCODE_ATAN: return( "ATAN" ); break;
		case OPCODE_POS: return( "POS" ); break;
		case OPCODE_RND: return( "RND" ); break;
		case OPCODE_FOR: return( "FOR" ); break;
		case OPCODE_REV: return( "REV" ); break;
		case OPCODE_ROT: return( "ROT" ); break;
		case OPCODE_LSR: return( "LSR" ); break;
		case OPCODE_RDR: return( "RDR" ); break;
		case OPCODE_LNC: return( "LNC" ); break;
		case OPCODE_NOS: return( "NOS" ); break;
		case OPCODE_SHD: return( "SHD" ); break;
		case OPCODE_POW: return( "POW" ); break;
		case OPCODE_DAM: return( "DAM" ); break;
		case OPCODE_HED: return( "HED" ); break;
		case OPCODE_STP: return( "STP" ); break;
		case OPCODE_MIS: return( "MIS" ); break;
		case OPCODE_TIC: return( "TIC" ); break;
		case OPCODE_WAL: return( "WAL" ); break;
		case OPCODE_PRR: return( "PRR" ); break;
		case OPCODE_PRS: return( "PRS" ); break;
		case OPCODE_PSH: return( "PSH" ); break;
		case OPCODE_POP: return( "POP" ); break;
		case OPCODE_CMP: return( "CMP" ); break;
		case OPCODE_LDR: return( "LDR" ); break;
		case OPCODE_STR: return( "STR" ); break;
		default:
			break;
	}
	return( "_ERR_" );
}
	
char *opflags( uint16_t n ) {
	switch( n ) {
		case 0: return( "" ); break;
		case IFLAG_GT: return( "GT" ); break;
		case IFLAG_GT|IFLAG_LT: return( "NE" ); break;
		case IFLAG_LT: return( "LT" ); break;
		case IFLAG_GT|IFLAG_EQ: return( "GTE" ); break;
		case IFLAG_LT|IFLAG_EQ: return( "LTE" ); break;
		case IFLAG_EQ: return( "EQ" ); break;
		case IFLAG_EQ|IFLAG_GT|IFLAG_LT: return( "NV" ); break;
		case IFLAG_S: return( "S" ); break;
		case IFLAG_S|IFLAG_GT: return( "GTS" ); break;
		case IFLAG_S|IFLAG_GT|IFLAG_LT: return( "NES" ); break;
		case IFLAG_S|IFLAG_LT: return( "LTS" ); break;
		case IFLAG_S|IFLAG_GT|IFLAG_EQ: return( "GTES" ); break;
		case IFLAG_S|IFLAG_LT|IFLAG_EQ: return( "LTES" ); break;
		case IFLAG_S|IFLAG_EQ: return( "EQS" ); break;
		case IFLAG_S|IFLAG_EQ|IFLAG_GT|IFLAG_LT: return( "NVS" ); break;
		default: break;
	}
	return( "_ERR_" );
}

char *optype( uint16_t n ) {
	switch ( n ) {
		case IOPTYPE_NONE: return( "none" ); break;
		case IOPTYPE_REGI: return( "RegI" ); break;
		case IOPTYPE_REGF: return( "RegF" ); break;
		case IOPTYPE_ABS: return( "Absolute" ); break;
		case IOPTYPE_NOS: return( "Number" ); break;
		default: break;
	}
	return( "_unknown_" );
}

char *opmode( uint16_t n ) {
	switch( n ) {
		case IAMODE_UNSET: return ( "Unset" ); break;
		case IAMODE_DIRECT: return ( "Direct" ); break;
		case IAMODE_ABSOLUTE: return ( "Absolute" ); break;
		case IAMODE_NINDEX: return ( "NIndex" ); break;
		case IAMODE_REGINDEX: return ( "RegIndex" ); break;
		case IAMODE_MEMADDR: return ( "MemAddr" ); break;
		default: break;
	}
	return( "Unknown" );
}

int32_t instr_decode( INSTRUCTION *i )
{
	uint32_t uval;
	uval=i->i_image;
	i->i_opcode=(uint16_t)(uval>>25);
	if( i->i_opcode >= OPCODE_LIMIT_NOADDRMODE ) {
		i->i_amode = ((i->i_opcode-OPCODE_LIMIT_NOADDRMODE)%6);
		i->i_opcode -= i->i_amode;
	} else {
		i->i_amode = IAMODE_DIRECT;
	}
	i->i_flags=(uval>>21)&0xf;
	i->i_op1=(uval>>14)&0x7f;
	i->i_op2=(uval>>7)&0x7f;
	i->i_op3=(uval)&0x7f;
}

int32_t instr_validate( INSTRUCTION *i, uint32_t pass )
{
	uint32_t ival;
	uint32_t bits;
	/* validate and compile instruction */
if( 1 ) {
	printf( "IV: opcode=%s(%hu) flags=%s(%hu) amode=%u", opcode( i->i_opcode ), i->i_opcode, opflags( i->i_flags), i->i_flags, i->i_amode );
	if( i->i_optype1 != IOPTYPE_NONE ) {
		printf( " op1=%s (%u)", optype( i->i_optype1 ), i->i_op1 );
		if( i->i_optype2 != IOPTYPE_NONE ) {
			printf( " op2=%s (%u)", optype( i->i_optype2 ), i->i_op2 );
			if( i->i_optype3 != IOPTYPE_NONE ) {
				printf( " op3=%s (%u)", optype( i->i_optype3 ), i->i_op3 );
			}
		}
	}
	printf( "\n" );
}

	switch( i->i_opcode ) {

	/* no operands */
		case OPCODE_STP:
		case OPCODE_NOP:
			if( i->i_optype1 != IOPTYPE_NONE ) {
				fprintf( stderr, "ras: error: invalid operands for %s instruction.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

	/* one register */
		case OPCODE_PRR:
		case OPCODE_FOR:
		case OPCODE_REV:
		case OPCODE_ROT:
		case OPCODE_LSR:
		case OPCODE_LNC:
		case OPCODE_HED:
			if( i->i_amode != IAMODE_DIRECT ) {
				fprintf( stderr, "ras: error: invalid address mode for %u instruction %s.\n",
					i->i_amode, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != IOPTYPE_REGI && i->i_optype1 != IOPTYPE_REGF ) {
				fprintf( stderr, "ras: error: invalid operand for %s instruction.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

	/* one integer register */
		case OPCODE_WAL:
		case OPCODE_TIC:
		case OPCODE_MIS:
		case OPCODE_NOS:
		case OPCODE_SHD:
		case OPCODE_POW:
		case OPCODE_DAM:
			if( i->i_amode != IAMODE_DIRECT ) {
				fprintf( stderr, "ras: error: invalid address mode for %u instruction %s.\n",
					i->i_amode, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != IOPTYPE_REGI ) {
				fprintf( stderr, "ras: error: invalid operand for %s instruction.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

	/* one operand, integer register or address */
		case OPCODE_PRS:
			if( i->i_amode != IAMODE_DIRECT ) {
				fprintf( stderr, "ras: error: invalid address mode for %u instruction %s.\n",
					i->i_amode, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != IOPTYPE_REGI && i->i_optype1 != IOPTYPE_ABS ) {
				fprintf( stderr, "ras: error: invalid operand for %s instruction.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

	/* one operand, any addressmode */
		case OPCODE_BLI:
		case OPCODE_JMP:
			/* These two can have all types of address modes except ABSOLUTE */
			if( i->i_amode == IAMODE_UNSET || i->i_amode == IAMODE_ABSOLUTE ) {
				fprintf( stderr, "ras: error: invalid address mode for %u instruction %s.\n",
					i->i_amode, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_DIRECT && ( i->i_optype1 != IOPTYPE_REGI || i->i_optype2 != IOPTYPE_NONE ) ) {
				fprintf( stderr, "ras: error: invalid operands for IAMODE_DIRECT on instruction %s.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_NINDEX && ( i->i_optype1 != IOPTYPE_REGI || i->i_optype2 != IOPTYPE_NOS ) ) {
				fprintf( stderr, "ras: error: invalid operands for IAMODE_NINDEX on instruction %s.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_REGINDEX && ( i->i_optype1 != IOPTYPE_REGI || i->i_optype2 != IOPTYPE_REGI ) ) {
				fprintf( stderr, "ras: error: invalid operands for IAMODE_REGINDEX on instruction %s.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_MEMADDR && i->i_optype1 != IOPTYPE_ABS ) {
				fprintf( stderr, "ras: error: invalid operands for IAMODE_MEMADDR on instruction %s.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

	/* two registers, both regi or regf */
		case OPCODE_SQR:
		case OPCODE_SIN:
		case OPCODE_COS:
		case OPCODE_TAN:
		case OPCODE_ASIN:
		case OPCODE_ACOS:
		case OPCODE_ATAN:
		case OPCODE_POS:
			if( i->i_amode != IAMODE_DIRECT ) {
				fprintf( stderr, "ras: error: invalid address mode for %u instruction %s.\n",
					i->i_amode, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != IOPTYPE_REGI && i->i_optype1 != IOPTYPE_REGF ) {
				fprintf( stderr, "ras: error: invalid operand type %u,%u instruction %s.\n",
					i->i_optype1, i->i_optype2, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != i->i_optype2 ) {
				fprintf( stderr, "ras: error: unmatched type %u,%u instruction %s.\n",
					i->i_optype1, i->i_optype2, opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

	/* two registers, both regi */
		case OPCODE_NOT:
		case OPCODE_RND:
			if( i->i_amode != IAMODE_DIRECT ) {
				fprintf( stderr, "ras: error: invalid address mode for %u instruction %s.\n",
					i->i_amode, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != IOPTYPE_REGI || i->i_optype2 != IOPTYPE_REGI ) {
				fprintf( stderr, "ras: error: invalid operand type %u,%u instruction %s.\n",
					i->i_optype1, i->i_optype2, opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

	/* two registers and one nos */
		case OPCODE_ROR:
		case OPCODE_ROL:
		case OPCODE_SFR:
		case OPCODE_SFL:
			if( i->i_amode != IAMODE_DIRECT ) {
				fprintf( stderr, "ras: error: invalid address mode for %u instruction %s.\n",
					i->i_amode, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != IOPTYPE_REGI || i->i_optype2 != IOPTYPE_REGI ) {
				fprintf( stderr, "ras: error: invalid first/second operand type %u instruction %s.\n",
					i->i_optype1, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype3 != IOPTYPE_NOS ) {
				fprintf( stderr, "ras: error: invalid third operand type %u instruction %s.\n",
					i->i_optype1, opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

	/* three integer registers */
		case OPCODE_AND:
		case OPCODE_OR:
		case OPCODE_XOR:
			if( i->i_optype1 != IOPTYPE_REGI ) {
				fprintf( stderr, "ras: error: invalid first operand type %u instruction %s.\n",
					i->i_optype1, opcode( i->i_opcode ) );
				return( 0 );
			}
			/* fall through to same checks for MUL/DIV/SUB/ADD */
		case OPCODE_MUL:
		case OPCODE_DIV:
		case OPCODE_SUB:
		case OPCODE_ADD:
			/* These must have rX/fX as all 3 operands, and only in direct amode */
			if( i->i_amode != IAMODE_DIRECT ) {
				fprintf( stderr, "ras: error: invalid address mode for %u instruction %s.\n",
					i->i_amode, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != IOPTYPE_REGI && i->i_optype1 != IOPTYPE_REGF ) {
				fprintf( stderr, "ras: error: invalid first operand type %u instruction %s.\n",
					i->i_optype1, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != i->i_optype2 ) {
				fprintf( stderr, "ras: error: operand 1,2 mismatch %u != %u instruction %s.\n",
					i->i_optype1, i->i_optype2, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype2 != i->i_optype3 ) {
				fprintf( stderr, "ras: error: operands 2,3 mismatch %u != %u instruction %s.\n",
					i->i_optype2, i->i_optype3, opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;
		case OPCODE_RDR:
			if( i->i_optype1 != IOPTYPE_REGI && i->i_optype1 != IOPTYPE_REGF ) {
				fprintf( stderr, "ras: error: invalid first operand type %u instruction %s.\n",
					i->i_optype1, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype2 != IOPTYPE_REGI ) {
				fprintf( stderr, "ras: error: invalid second operand type %u instruction %s.\n",
					i->i_optype2, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype3 != IOPTYPE_REGI && i->i_optype3 != IOPTYPE_REGF ) {
				fprintf( stderr, "ras: error: invalid third operand type %u instruction %s.\n",
					i->i_optype3, opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

	/* one register then any addressing mode */
		case OPCODE_CMP:
		case OPCODE_LDR:
		case OPCODE_POP:
		case OPCODE_PSH:
		case OPCODE_STR:
			/* These must have rX/fX as the first operand, then any of the modes */
			if( i->i_amode == IAMODE_UNSET ) {
				fprintf( stderr, "ras: error: invalid address mode for %u instruction %s.\n",
					i->i_amode, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_optype1 != IOPTYPE_REGI && i->i_optype1 != IOPTYPE_REGF ) {
				fprintf( stderr, "ras: error: invalid first operand type %u instruction %s.\n",
					i->i_optype1, opcode( i->i_opcode ) );
				return( 0 );
			}
			/* allow conversion from REGI to REGF and back */
			if( i->i_amode == IAMODE_DIRECT && i->i_optype2 != IOPTYPE_REGF && i->i_optype2 != IOPTYPE_REGI ) {
				fprintf( stderr, "ras: error: invalid DIRECT operands types %u,%u instruction %s.\n",
					i->i_optype2, i->i_optype3, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_DIRECT && i->i_optype3 != IOPTYPE_NONE ) {
				fprintf( stderr, "ras: error: invalid DIRECT operands types %u,%u instruction %s.\n",
					i->i_optype2, i->i_optype3, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_ABSOLUTE && i->i_opcode == OPCODE_POP ) {
				fprintf( stderr, "ras: error: invalid ABSOLUTE amode for instruction %s.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_ABSOLUTE && i->i_opcode == OPCODE_STR ) {
				fprintf( stderr, "ras: error: invalid ABSOLUTE amode for instruction %s.\n", opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_ABSOLUTE && ( i->i_optype1 != IOPTYPE_REGI || i->i_optype2 != IOPTYPE_NOS || i->i_optype3 != IOPTYPE_NONE ) ) {
				fprintf( stderr, "ras: error: invalid ABSOLUTE operands types %u,%u,%u instruction %s.\n",
					i->i_optype1, i->i_optype2, i->i_optype3, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_MEMADDR && ( i->i_optype2 != IOPTYPE_ABS || i->i_optype3 != IOPTYPE_NONE ) ) {
				fprintf( stderr, "ras: error: invalid MEMADDR operands types %u,%u instruction %s.\n",
					i->i_optype2, i->i_optype3, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_NINDEX && ( i->i_optype2 != IOPTYPE_REGI || i->i_optype3 != IOPTYPE_NOS ) ) {
				fprintf( stderr, "ras: error: invalid NINDEX operands types %u,%u instruction %s.\n",
					i->i_optype2, i->i_optype3, opcode( i->i_opcode ) );
				return( 0 );
			}
			if( i->i_amode == IAMODE_REGINDEX && ( i->i_optype2 != IOPTYPE_REGI || i->i_optype3 != IOPTYPE_REGI ) ) {
				fprintf( stderr, "ras: error: invalid REGINDEX operands types %u,%u instruction %s.\n",
					i->i_optype2, i->i_optype3, opcode( i->i_opcode ) );
				return( 0 );
			}
			/* ok */
			break;

		default:
			fprintf( stderr, "ras: error: unknown opcode %s (%u)\n", opcode( i->i_opcode ), i->i_opcode );
			return( 0 );
			break;
	}
	/* build instruction */
	ival=0;
	bits=0;
	ival=i->i_opcode;
	if( i->i_opcode < OPCODE_LIMIT_NOADDRMODE ) {
		if( i->i_amode != 1 ) {
			fprintf( stderr, "ras: error: invalid amode %u for opcode %s.\n", i->i_amode, opcode( i->i_opcode ) );
			return( 0 );
		}
	} else {
		/* address mode opcode, add it on */
		ival+=i->i_amode;
	}
	/* shift up 4 bits for flags */

	bits+=7;

	ival <<= 4;
	if( i->i_flags > 16 ) {
		fprintf( stderr, "ras: error: invalid flags %u for opcode %s.\n", i->i_flags, opcode( i->i_opcode ) );
		return( 0 );
	}
	ival |= i->i_flags; 
	bits+=4;

	switch( i->i_optype1 ) {
		case IOPTYPE_NONE:
			break;
		case IOPTYPE_REGI:
		case IOPTYPE_REGF:	/* reg number already set to 64+n */
			ival <<= 7; bits+=7;
			ival |= i->i_op1;
			break;
		case IOPTYPE_ABS:
			ival <<= 14; bits+=14;
			if( ( i->i_op1 % 4 ) != 0 ) {
				fprintf( stderr, "ras: error: address %u not aligned for operand opcode=%s.\n", i->i_op1, opcode( i->i_opcode ) );
				return( 0 );
			}
			ival |= ( i->i_op1 >> 2 );
			break;
		case IOPTYPE_NOS:
			ival <<= 7; bits+=7;
			if( i->i_op1 >= 127 ) {
				fprintf( stderr, "ras: error: number %u too large ( >127 ) for operand opcode=%s.\n", i->i_op1, opcode( i->i_opcode ) );
				return( 0 );
			}
			ival |= i->i_op1;
			break;
		default:
			fprintf( stderr, "ras: error: unknown operand type %u for opcode %s.\n", i->i_optype1, opcode( i->i_opcode ) );
			return( 0 );
			break;
	}
	switch( i->i_optype2 ) {
		case IOPTYPE_NONE:
			break;
		case IOPTYPE_REGI:
		case IOPTYPE_REGF:	/* reg number already set to 64+n */
			ival <<= 7; bits+=7;
			ival |= i->i_op2;
			break;
		case IOPTYPE_ABS:
			ival <<= 14; bits+=14;
			if( ( i->i_op2 % 4 ) != 0 ) {
				fprintf( stderr, "ras: error: address %u not aligned for operand opcode=%s.\n", i->i_op2, opcode( i->i_opcode ) );
				return( 0 );
			}
			ival |= ( i->i_op2 >> 2 );
			break;
		case IOPTYPE_NOS:
			ival <<= 7; bits+=7;
			if( i->i_op2 >= 127 ) {
				fprintf( stderr, "ras: error: number %u too large ( >127 ) for operand opcode=%s.\n", i->i_op2, opcode( i->i_opcode ) );
				return( 0 );
			}
			ival |= i->i_op2;
			break;
		default:
			fprintf( stderr, "ras: error: unknown operand type %u for opcode %s.\n", i->i_optype2, opcode( i->i_opcode ) );
			return( 0 );
			break;
	}
	switch( i->i_optype3 ) {
		case IOPTYPE_NONE:
			break;
		case IOPTYPE_REGI:
		case IOPTYPE_REGF:	/* reg number already set to 64+n */
			ival <<= 7; bits+=7;
			ival |= i->i_op3;
			break;
		case IOPTYPE_ABS:
			fprintf( stderr, "ras: error: invalid 3rd operand ABS for opcode=%s.\n", opcode( i->i_opcode ) );
			return( 0 );
			break;
		case IOPTYPE_NOS:
			ival <<= 7; bits+=7;
			if( i->i_op3 >= 127 ) {
				fprintf( stderr, "ras: error: number %u too large ( >127 ) for operand opcode=%s.\n", i->i_op3, opcode( i->i_opcode ) );
				return( 0 );
			}
			ival |= i->i_op3;
			break;
		default:
			fprintf( stderr, "ras: error: unknown operand type %u for opcode %s.\n", i->i_optype1, opcode( i->i_opcode ) );
			return( 0 );
			break;
	}
	if( bits < 32 ) {
		ival <<= (32-bits);
		bits=32;
	}
	if( pass == 2 ) {
		printf( "instruction = %8x (bits=%u)\n", ival, bits );
	}
	if( bits > 32 ) {
		fprintf( stderr, "ras: error: invalid instruction built with %u bits for opcode %s.\n", bits, opcode( i->i_opcode ) );
		return( 0 );
	}
	if( pass == 3 ) {
		/* Verify value in i->i_image */
		if( ival != i->i_image ) {
			fprintf( stderr, "ras: error: rebuilt instruction %u doesn't match image %u for opcode %s.\n",
			ival, i->i_image, opcode( i->i_opcode ) );
			return( 0 );
		}
	} else {
		i->i_image=ival;
	}
	return( 1 );
}

int32_t instr_parse_oper( char *text, INSTRUCTION *i, uint32_t pass )
{
	int f,t;
	int ok;
	int tmp,tmpa;
	char *sqbracket;
	uint32_t utmp,utmpa;
	uint32_t opmode;
	uint16_t oper1,oper2;
	uint16_t oper1type,oper2type;
	uint16_t nos_oper;

	opmode=IAMODE_UNSET;
	nos_oper=0;
	oper1type=IOPTYPE_NONE;
	oper2type=IOPTYPE_NONE;

	ok=1;
	/* remove all whitespace */

	f=0;
	t=0;
	while( text[f] ) {
		if( ! isspace( text[f] ) ) {
			text[t++]=text[f];
		}
		f++;
	}
	text[t]='\0';

	/* Can't be blank */

	/* printf( "IPO: '%s'\n", text ); */

	if( strcmp( text, "pc" ) == 0 ) {
		opmode=IAMODE_DIRECT;
		oper1=REG_PC;
		oper1type=IOPTYPE_REGI;
		nos_oper=1;
	} else if( strcmp( text, "lr" ) == 0 ) {
		opmode=IAMODE_DIRECT;
		oper1=REG_LR;
		oper1type=IOPTYPE_REGI;
		nos_oper=1;
	} else if( sscanf( text, "#%u", &utmp ) == 1 ) {
		opmode=IAMODE_ABSOLUTE;
		oper1=utmp;
		oper1type=IOPTYPE_NOS;
		nos_oper=1;
	} else if( sscanf( text, "%u", &utmp ) == 1 ) {
		opmode=IAMODE_MEMADDR;
		oper1=utmp;
		oper1type=IOPTYPE_ABS;
		nos_oper=1;
	} else if( sscanf( text, "r%u[r%u]", &utmp, &utmpa ) == 2 ) {
		if( utmp > REG_NOS_MAX ) {
			fprintf( stderr, "ras: error: invalid reg nos '%s'\n", text );
			return( 0 );
		}
		if( utmpa > REG_NOS_MAX ) {
			fprintf( stderr, "ras: error: invalid reg nos '%s'\n", text );
			return( 0 );
		}
		opmode=IAMODE_NINDEX;
		oper1=REG_INT_BASE+utmp;
		oper1type=IOPTYPE_REGI;
		oper2=utmpa>>2;
		oper2type=IOPTYPE_NOS;
		nos_oper=2;
	} else if( sscanf( text, "r%u[%u]", &utmp, &utmpa ) == 2 ) {
		if( utmp > REG_NOS_MAX ) {
			fprintf( stderr, "ras: error: invalid reg nos '%s'\n", text );
			return( 0 );
		}
		if( utmpa >= 508 ) {
			fprintf( stderr, "ras: error: invalid offset index '%s' (max=508)\n", text );
			return( 0 );
		}
		opmode=IAMODE_NINDEX;
		oper1=REG_INT_BASE+utmp;
		oper1type=IOPTYPE_REGI;
		oper2=utmpa>>2;
		oper2type=IOPTYPE_NOS;
		nos_oper=2;
	} else if( sscanf( text, "f%u", &utmp ) == 1 ) {
		opmode=IAMODE_DIRECT;
		if( utmp <= REG_NOS_MAX ) {
			oper1=REG_FLT_BASE+utmp;
			oper1type=IOPTYPE_REGF;
			nos_oper=1;
		} else {
			fprintf( stderr, "ras: error: invalid reg nos '%s'\n", text );
			return( 0 );
		}
	} else if( sscanf( text, "r%u", &utmp ) == 1 ) {
		opmode=IAMODE_DIRECT;
		if( utmp <= REG_NOS_MAX ) {
			oper1=REG_INT_BASE+utmp;
			oper1type=IOPTYPE_REGI;
			nos_oper=1;
		} else {
			fprintf( stderr, "ras: error: invalid reg nos '%s'\n", text );
			return( 0 );
		}
	} else {
		/* assume it is a label */
		if( pass == 1 ) {
			/* tighter check will be performed on pass=2 */
			opmode=IAMODE_MEMADDR;
			oper1=0;
			oper1type=IOPTYPE_ABS;
			nos_oper=1;
		} else {
			if( hash_exists( text ) ) {
				opmode=IAMODE_MEMADDR;
				oper1=hash_get( text );
				oper1type=IOPTYPE_ABS;
				nos_oper=1;
			} else {
				fprintf( stderr, "ras: error: unknown label '%s'\n", text );
				return( 0 );
			}
		}
	}

	/* Fit operand(s) into INSTRUCTION structure */
	if( nos_oper == 1 ) {
		if( i->i_optype1 == IOPTYPE_NONE ) {
			i->i_amode=opmode;
			i->i_optype1=oper1type;
			i->i_op1=oper1;
		} else if( i->i_optype2 == IOPTYPE_NONE ) {
			i->i_amode=opmode;
			i->i_optype2=oper1type;
			i->i_op2=oper1;
		} else if( i->i_optype3 == IOPTYPE_NONE ) {
			i->i_optype3=oper1type;
			i->i_op3=oper1;
		} else {
			fprintf( stderr, "ras: error: invalid operands for instruction %s\n", opcode( i->i_opcode ) );
			return( 0 );
		}
	} else if( nos_oper == 2 ) {
		/* must already have one operand */
		if( i->i_optype1 == IOPTYPE_NONE ) {
			fprintf( stderr, "ras: error: invalid operands for instruction %s\n", opcode( i->i_opcode ) );
			return( 0 );
		}
		if( i->i_optype2 == IOPTYPE_NONE ) {
			i->i_amode=opmode;	/* over-ride with proper addressing mode */
			i->i_optype2=oper1type;
			i->i_op2=oper1;
			i->i_optype3=oper2type;
			i->i_op3=oper2;
		} else {
			fprintf( stderr, "ras: error: invalid operands for instruction %s\n", opcode( i->i_opcode ) );
			return( 0 );
		}
	}

	return( 1 );
}

int32_t instr_parse( char *text, INSTRUCTION *i, uint32_t pass )
{
	int ok=1;
	int ct;
	char *curr,*tmp;

	bzero( i, sizeof(INSTRUCTION) );

	curr=text;

	/* whitespace should have been removed from beginning of the line */
	ct=0;
	ok=0;
	while( opcode_array[ct].text[0] != 'Z' ) {
		if( strncmp( curr, opcode_array[ct].text, opcode_array[ct].len ) == 0 ) {
			i->i_opcode=opcode_array[ct].opcode;
			curr+=opcode_array[ct].len;
			ok=1;
			break;
		}
		ct++;
	}
	if( ok == 0 ) {
		fprintf( stderr, "ras: error: unknown opcode: %s\n", text );
		return( 0 );
	}
	if( *curr == 'S' ) {
		i->i_flags |= IFLAG_S;
		curr++;
	}
	if( *curr && isupper( *curr ) ) {
		ok=0;
		ct=0;
		while( flag_array[ct].text[0] != 'Z' ) {
			if( strncmp( curr, flag_array[ct].text, flag_array[ct].len ) == 0 ) {
				i->i_flags |= flag_array[ct].opcode;
				curr+=flag_array[ct].len;
				ok=1;
				break;
			}
			ct++;
		}
	}
	if( *curr == 'S' ) {
		if( i->i_flags && IFLAG_S ) {
			fprintf( stderr, "ras: warning: S flag set twice on line: %s\n", text );
		}
		i->i_flags |= IFLAG_S;
		curr++;
	}
	if( ! *curr ) {
		/* empty line, no operands */
		/* parsed but may not be syntactically correct */
		if( instr_validate( i, pass ) != 1 ) {
			return( 0 );
		}
		return( 1 );
	}
	/* if there's anything else on the line it must be separated
		by whitespace */
	if( ! isspace( *curr ) ) {
		fprintf( stderr, "ras: error: extra chars at end of opcode: %s [%s]\n", text, curr );
		return( 0 );
	}
	while( isspace( *curr ) ) {
		curr++;
	}
	if( ! *curr ) {
		/* empty line, no operands */
		/* parsed but may not be syntactically correct */
		if( instr_validate( i, pass ) != 1 ) {
			return( 0 );
		}
		return( 1 );
	}
	/* first operand */
	tmp=strchr( curr, ',' );
	if( tmp ) {
		*tmp='\0';
		tmp++;
	}
	/* parse oper 1 */
	if( instr_parse_oper( curr, i, pass ) != 1 ) {
		return( 0 );
	}
	if( ! tmp ) {
		if( instr_validate( i, pass ) != 1 ) {
			return( 0 );
		}
		return( 1 );
	}
	/* second operand */
	curr=tmp;
	tmp=strchr( curr, ',' );
	if( tmp ) {
		*tmp='\0';
		tmp++;
	}
	/* parse next oper */
	if( instr_parse_oper( curr, i, pass  ) != 1 ) {
		return( 0 );
	}
	if( ! tmp ) {
		if( instr_validate( i, pass ) != 1 ) {
			return( 0 );
		}
		return( 1 );
	}
	/* final operand */
	curr=tmp;
	/* parse next oper */
	if( instr_parse_oper( curr, i, pass ) != 1 ) {
		return( 0 );
	}
	if( instr_validate( i, pass ) != 1 ) {
		return( 0 );
	}
	return( 1 );
	/* Complain about unknown strings only if pass=2 */
}
