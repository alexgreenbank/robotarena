/*
 * arena
 */

#define ARENA_ERR	0
#define ARENA_OK	1

#define ARENA_X 480
#define ARENA_Y 320
#define MAX_ROBOTS	5
#define MAX_MISSILES	MAX_ROBOTS

#define MAX_MEMORY	65536

#define RADAR_WALL	0
#define RADAR_ROBOT	1
#define RADAR_MISSILE	2
#define RADAR_BLAST	3

#define STATE_INIT	0
#define STATE_RUN	1
#define STATE_STOP	2
#define STATE_ABEND	3
#define STATE_MEMFAULT	4
#define STATE_DEAD	5

#define REG_NOS_MAX	63

#define REG_INT_BASE	0
#define REG_FLT_BASE	64

#define REG_PC		0
#define REG_LR		1

#define IAMODE_UNSET	0		/* unset on not applicable for single amode instructions */
#define IAMODE_DIRECT	1		/* LDR r1,r2 aka MOV r1,r2 */ /* r1=r2 */
#define IAMODE_ABSOLUTE	2		/* LDR r1,#123 */ /* r1=123 */
#define IAMODE_NINDEX	3		/* LDR r1,r2[1] or LDR r1,r2[0] */
					/* ... r1=mem(r2+4) or r1=mem(r2) */
#define IAMODE_REGINDEX	4		/* LDR r1,r2[r3] */ /* r1=mem(r2+4r3) */
#define IAMODE_MEMADDR	5		/* LDR r1,124 */ /* r1=mem(124) */

#define IFLAG_ALL	0
#define IFLAG_LT	1
#define IFLAG_EQ	2
#define IFLAG_LTE	3
#define IFLAG_GT	4
#define IFLAG_NE	5
#define IFLAG_GTE	6
#define IFLAG_NV	7
#define IFLAG_S		8

#define IOPTYPE_NONE	0
#define IOPTYPE_REGI	1
#define IOPTYPE_REGF	2
#define IOPTYPE_ABS	3
#define IOPTYPE_NOS	4

/*
 * Opcodes below OPCODE_LIMIT_NOADDRMODE are all IAMODE_DIRECT
 * i.e. they only ever use registers
 */

#define OPCODE_NOP	0
#define OPCODE_MUL	1
#define OPCODE_DIV	2
#define OPCODE_SUB	3
#define OPCODE_ADD	4
#define OPCODE_AND	5
#define OPCODE_OR	6
#define OPCODE_XOR	7
#define OPCODE_NOT	8
#define OPCODE_ROL	9
#define OPCODE_ROR	10
#define OPCODE_SFL	11
#define OPCODE_SFR	12
#define OPCODE_SQR	13
#define OPCODE_SIN	14
#define OPCODE_COS	15
#define OPCODE_TAN	16
#define OPCODE_ASIN	17
#define OPCODE_ACOS	18
#define OPCODE_ATAN	19
#define OPCODE_POS	20
#define OPCODE_RND	21
#define OPCODE_FOR	22
#define OPCODE_REV	23
#define OPCODE_ROT	24
#define OPCODE_LSR	25
#define OPCODE_LNC	26
#define OPCODE_NOS	27
#define OPCODE_SHD	28
#define OPCODE_DAM	29
#define OPCODE_HED	30
#define OPCODE_STP	31
#define OPCODE_MIS	32
#define OPCODE_TIC	33
#define OPCODE_WAL	34
#define OPCODE_PRR	35		/* print reg */
#define OPCODE_PRS	36		/* print string */
#define OPCODE_POW	37
#define OPCODE_RDR	38

#define OPCODE_LIMIT_NOADDRMODE	86

#define OPCODE_PSH	86
#define OPCODE_POP	92
#define OPCODE_BLI	98
#define OPCODE_JMP	104
#define OPCODE_CMP	110
#define OPCODE_LDR	116
#define OPCODE_STR	122

#define OPCODE_LIMIT_MAX	127

#define SHIELD_OFF	0
#define SHIELD_ON	1

#define MAX_FUEL	200
#define MAX_HEALTH	9
#define DAMAGE_CT	20

#define MISSILE_RUN	1
#define MISSILE_EXPLOSION	2
#define MISSILE_DEAD	3

typedef struct instruction {
	uint32_t	i_image;
	uint16_t	i_opcode;
	uint16_t	i_flags;
	uint16_t	i_amode;
	uint16_t	i_optype1;
	uint16_t	i_optype2;
	uint16_t	i_optype3;
	uint16_t	i_op1;
	uint16_t	i_op2;
	uint16_t	i_op3;
} INSTRUCTION;

typedef struct robot {
	uint32_t	r_id;
	uint32_t	r_state;
	uint32_t	r_pc;		/* at what point it died - to control ties */
	int32_t		r_for_dist;	/* distance we're trying to move */
	int32_t		r_for_ct;	/* distance we've moved so far */
	int32_t		r_rot_ct;
	uint32_t	r_damage_ct;	/* inactivity after being hit by missile or collision with wall/robot */
	uint32_t	r_health;
	uint32_t	r_x,r_y,r_head;
	uint32_t	r_size;
	uint32_t	r_fuel;		/* shield fuel */
	uint32_t	r_shield;	/* shields on/off */
	int32_t		r_regi[64];
	float		r_regf[64];
	unsigned char	r_memory[MAX_MEMORY];
	uint8_t		r_flags;
	struct robot	*r_next;
	struct missile	*r_missile;
} ROBOT;

typedef struct missile {
	uint32_t	m_lx,m_ly,m_lang,m_range;
	double		m_ex,m_ey;	/* explosion coordinates */
	uint32_t	m_state;	/* moving or exploding */
	uint32_t	m_ct;
	struct missile	*m_next;
	struct robot	*m_robot;
} MISSILE;
