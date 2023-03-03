/* ferguson-srb1-emu                                                     */
/*=======================================================================*/
/* Copyright 2023 Philip Heron <phil@sanslogic.co.uk>                    */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cpu_65c02.h"

enum _addr_mode_t {
	_invalid,
	_implicit,
	_a,
	_immediate,
	_absolute,
	_absolute_x,
	_absolute_y,
	_relative,
	_zp,
	_zp_x,
	_zp_y,
	_indirect,
	_zp_indirect,
	_zp_indirect_x,
	_indirect_x,
	_zp_indirect_y,
	_zp_relative,
};

#define _C (1 << 0)
#define _Z (1 << 1)
#define _I (1 << 2)
#define _D (1 << 3)
#define _B (1 << 4)
#define _V (1 << 6)
#define _N (1 << 7)

struct _instr_t {
	const char *m;
	enum _addr_mode_t mode;
	uint8_t l;
	uint8_t mcycles;
	uint8_t flags;
};

static const struct _instr_t _instrs[0x100] =
{
	/* 0x0x */
	{ "BRK",  _immediate,     2, 7 },
	{ "ORA",  _zp_indirect_x, 2, 6, _Z | _N },
	{ "",     _invalid,       2, 2 },
	{ "",     _invalid,       1, 1 },
	{ "TSB",  _zp,            2, 5, _Z },
	{ "ORA",  _zp,            2, 3, _Z | _N },
	{ "ASL",  _zp,            2, 5, _Z | _N | _C },
	{ "RMB0", _zp,            2, 5 },
	{ "PHP",  _implicit,      1, 3 },
	{ "ORA",  _immediate,     2, 2, _Z | _N },
	{ "ASL",  _a,             1, 2, _Z | _N | _C },
	{ "",     _invalid,       1, 1 },
	{ "TSB",  _absolute,      3, 6, _Z },
	{ "ORA",  _absolute,      3, 4, _Z | _N },
	{ "ASL",  _absolute,      3, 6, _Z | _N | _C },
	{ "BBR0", _zp_relative,   3, 4 },
	
	/* 0x1x */
	{ "BPL",  _relative,      2, 2 },
	{ "ORA",  _zp_indirect_y, 2, 5, _Z | _N },
	{ "ORA",  _zp_indirect,   2, 5, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "TRB",  _zp,            2, 5 },
	{ "ORA",  _zp_x,          2, 4, _Z | _N },
	{ "ASL",  _zp_x,          2, 6, _Z | _N | _C },
	{ "RMB1", _zp,            2, 5 },
	{ "CLC",  _implicit,      1, 2 },
	{ "ORA",  _absolute_y,    3, 4, _Z | _N },
	{ "INC",  _a,             1, 2, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "TRB",  _absolute,      3, 6, _Z },
	{ "ORA",  _absolute_x,    3, 4, _Z | _N },
	{ "ASL",  _absolute_x,    3, 6, _Z | _N | _C },
	{ "BBR1", _zp_relative,   3, 4 },
	
	/* 0x2x */
	{ "JSR",  _absolute,      3, 6 },
	{ "AND",  _zp_indirect_x, 2, 6, _Z | _N },
	{ "",     _invalid,       2, 2 },
	{ "",     _invalid,       1, 1 },
	{ "BIT",  _zp,            2, 3, _Z },
	{ "AND",  _zp,            2, 3, _Z | _N },
	{ "ROL",  _zp,            2, 5, _Z | _N | _C },
	{ "RMB2", _zp,            2, 5 },
	{ "PLP",  _implicit,      1, 4 },
	{ "AND",  _immediate,     2, 2, _Z | _N },
	{ "ROL",  _a,             1, 2, _Z | _N | _C },
	{ "",     _invalid,       1, 1 },
	{ "BIT",  _absolute,      3, 4, _Z },
	{ "AND",  _absolute,      3, 4, _Z | _N },
	{ "ROL",  _absolute,      3, 6, _Z | _N | _C },
	{ "BBR2", _zp_relative,   3, 4 },
	
	/* 0x3x */
	{ "BMI",  _relative,      2, 2 },
	{ "AND",  _zp_indirect_y, 2, 5, _Z | _N },
	{ "AND",  _zp_indirect,   2, 5, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "BIT",  _zp_x,          2, 4, _Z },
	{ "AND",  _zp_x,          2, 4, _Z | _N },
	{ "ROL",  _zp_x,          2, 6, _Z | _N | _C },
	{ "RMB3", _zp,            2, 5 },
	{ "SEC",  _implicit,      1, 2 },
	{ "AND",  _absolute_y,    3, 4, _Z | _N },
	{ "DEC",  _a,             1, 2, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "BIT",  _absolute_x,    3, 4, _Z },
	{ "AND",  _absolute_x,    3, 4, _Z | _N },
	{ "ROL",  _absolute_x,    3, 6, _N | _Z | _C },
	{ "BBR3", _zp_relative,   3, 4 },
	
	/* 0x4x */
	{ "RTI",  _implicit,      1, 6 },
	{ "EOR",  _zp_indirect_x, 2, 6, _Z | _N },
	{ "",     _invalid,       2, 2 },
	{ "",     _invalid,       1, 1 },
	{ "",     _invalid,       2, 3 },
	{ "EOR",  _zp,            2, 3, _Z | _N },
	{ "LSR",  _zp,            2, 5, _Z | _N | _C },
	{ "RMB4", _zp,            2, 5 },
	{ "PHA",  _implicit,      1, 3 },
	{ "EOR",  _immediate,     2, 2, _Z | _N },
	{ "LSR",  _a,             1, 2, _Z | _N | _C },
	{ "",     _invalid,       1, 1 },
	{ "JMP",  _absolute,      3, 3 },
	{ "EOR",  _absolute,      3, 4, _Z | _N },
	{ "LSR",  _absolute,      3, 6, _Z | _N | _C },
	{ "BBR4", _zp_relative,   3, 4 },
	
	/* 0x5x */
	{ "BVC",  _relative,      2, 2 },
	{ "EOR",  _zp_indirect_y, 2, 5, _Z | _N },
	{ "EOR",  _zp_indirect,   2, 5, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "",     _invalid,       2, 4 },
	{ "EOR",  _zp_x,          2, 4, _Z | _N },
	{ "LSR",  _zp_x,          2, 6, _Z | _N },
	{ "RMB5", _zp,            2, 5 },
	{ "CLI",  _implicit,      1, 2 },
	{ "EOR",  _absolute_y,    3, 4, _Z | _N },
	{ "PHY",  _implicit,      1, 3 },
	{ "",     _invalid,       1, 1 },
	{ "",     _invalid,       3, 8 },
	{ "EOR",  _absolute_x,    3, 4, _Z | _N },
	{ "LSR",  _absolute_x,    3, 6, _Z | _N | _C },
	{ "BBR5", _zp_relative,   3, 4 },
	
	/* 0x6x */
	{ "RTS",  _implicit,      1, 6 },
	{ "ADC",  _zp_indirect_x, 2, 6, _N | _Z | _C | _V },
	{ "",     _invalid,       2, 2 },
	{ "",     _invalid,       1, 1 },
	{ "STZ",  _zp,            2, 4 },
	{ "ADC",  _zp,            2, 3, _N | _Z | _C | _V },
	{ "ROR",  _zp,            2, 5, _Z | _N | _C },
	{ "RMB6", _zp,            2, 5 },
	{ "PLA",  _implicit,      1, 4, _N | _Z },
	{ "ADC",  _immediate,     2, 2, _N | _Z | _C | _V },
	{ "ROR",  _a,             1, 2, _Z | _N | _C },
	{ "",     _invalid,       1, 1 },
	{ "JMP",  _indirect,      3, 6 },
	{ "ADC",  _absolute,      3, 4, _N | _Z | _C | _V },
	{ "ROR",  _absolute,      3, 6, _Z | _N | _C },
	{ "BBR6", _zp_relative,   3, 4 },
	
	/* 0x7x */
	{ "BVS",  _relative,      2, 2 },
	{ "ADC",  _zp_indirect_y, 2, 5, _N | _Z | _C | _V },
	{ "ADC",  _zp_indirect,   2, 5, _N | _Z | _C | _V },
	{ "",     _invalid,       1, 1 },
	{ "STZ",  _zp_x,          2, 5 },
	{ "ADC",  _zp_x,          2, 4, _N | _Z | _C | _V },
	{ "ROR",  _zp_x,          2, 6, _Z | _N | _C },
	{ "RMB7", _zp,            2, 5 },
	{ "SEI",  _implicit,      1, 2 },
	{ "ADC",  _absolute_y,    3, 4, _N | _Z | _C | _V },
	{ "PLY",  _implicit,      1, 4, _N | _Z },
	{ "",     _invalid,       1, 1 },
	{ "JMP",  _indirect_x,    3, 6 },
	{ "ADC",  _absolute_x,    3, 4, _N | _Z | _C | _V },
	{ "ROR",  _absolute_x,    3, 6, _Z | _N | _C },
	{ "BBR7", _zp_relative,   3, 4 },
	
	/* 0x8x */
	{ "BRA",  _relative,      2, 3 },
	{ "STA",  _zp_indirect_x, 2, 7 },
	{ "",     _invalid,       2, 2 },
	{ "",     _invalid,       1, 1 },
	{ "STY",  _zp,            2, 4 },
	{ "STA",  _zp,            2, 4 },
	{ "STX",  _zp,            2, 4 },
	{ "SMB0", _zp,            2, 5 },
	{ "DEY",  _implicit,      1, 2, _N | _Z },
	{ "BIT",  _immediate,     2, 2, _Z },
	{ "TXA",  _implicit,      1, 2, _N | _Z },
	{ "",     _invalid,       1, 1 },
	{ "STY",  _absolute,      3, 5 },
	{ "STA",  _absolute,      3, 5 },
	{ "STX",  _absolute,      3, 5 },
	{ "BBS0", _zp_relative,   3, 4 },
	
	/* 0x9x */
	{ "BCC",  _relative,      2, 2 },
	{ "STA",  _zp_indirect_y, 2, 7 },
	{ "STA",  _zp_indirect,   2, 6 },
	{ "",     _invalid,       1, 1 },
	{ "STY",  _zp_x,          2, 5 },
	{ "STA",  _zp_x,          2, 5 },
	{ "STX",  _zp_y,          2, 5 },
	{ "SMB1", _zp,            2, 5 },
	{ "TYA",  _implicit,      1, 2, _Z | _N },
	{ "STA",  _absolute_y,    3, 6 },
	{ "TXS",  _implicit,      1, 2 },
	{ "",     _invalid,       1, 1 },
	{ "STZ",  _absolute,      3, 5 },
	{ "STA",  _absolute_x,    3, 6 },
	{ "STZ",  _absolute_x,    3, 6 },
	{ "BBS1", _zp_relative,   3, 4 },
	
	/* 0xAx */
	{ "LDY",  _immediate,     2, 2, _Z | _N },
	{ "LDA",  _zp_indirect_x, 2, 6, _Z | _N },
	{ "LDX",  _immediate,     2, 2, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "LDY",  _zp,            2, 3, _Z | _N },
	{ "LDA",  _zp,            2, 3, _Z | _N },
	{ "LDX",  _zp,            2, 3, _Z | _N },
	{ "SMB2", _zp,            2, 5 },
	{ "TAY",  _implicit,      1, 2, _Z | _N },
	{ "LDA",  _immediate,     2, 2, _Z | _N },
	{ "TAX",  _implicit,      1, 2, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "LDY",  _absolute,      3, 4, _Z | _N },
	{ "LDA",  _absolute,      3, 4, _Z | _N },
	{ "LDX",  _absolute,      3, 4, _Z | _N },
	{ "BBS2", _zp_relative,   3, 4 },
	
	/* 0xBx */
	{ "BCS",  _relative,      2, 2 },
	{ "LDA",  _zp_indirect_y, 2, 5, _Z | _N },
	{ "LDA",  _zp_indirect,   2, 5, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "LDY",  _zp_x,          2, 4, _Z | _N },
	{ "LDA",  _zp_x,          2, 4, _Z | _N },
	{ "LDX",  _zp_y,          2, 4, _Z | _N },
	{ "SMB3", _zp,            2, 5 },
	{ "CLV",  _implicit,      1, 2 },
	{ "LDA",  _absolute_y,    3, 4, _Z | _N },
	{ "TSX",  _implicit,      1, 2, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "LDY",  _absolute_x,    3, 4, _Z | _N },
	{ "LDA",  _absolute_x,    3, 4, _Z | _N },
	{ "LDX",  _absolute_y,    3, 4, _Z | _N },
	{ "BBS3", _zp_relative,   3, 4 },
	
	/* 0xCx */
	{ "CPY",  _immediate,     2, 2, _Z | _N | _C },
	{ "CMP",  _zp_indirect_x, 2, 6, _Z | _N | _C },
	{ "",     _invalid,       2, 2 },
	{ "",     _invalid,       1, 1 },
	{ "CPY",  _zp,            2, 3, _Z | _N | _C },
	{ "CMP",  _zp,            2, 3, _Z | _N | _C },
	{ "DEC",  _zp,            2, 5, _Z | _N },
	{ "SMB4", _zp,            2, 5 },
	{ "INY",  _implicit,      1, 2, _Z | _N },
	{ "CMP",  _immediate,     2, 2, _Z | _N | _C },
	{ "DEX",  _implicit,      1, 2, _Z | _N },
	{ "WAI",  _implicit,      1, 5 },
	{ "CPY",  _absolute,      3, 4, _Z | _N | _C },
	{ "CMP",  _absolute,      3, 4, _Z | _N | _C },
	{ "DEC",  _absolute,      3, 6, _Z | _N },
	{ "BBS4", _zp_relative,   3, 4 },
	
	/* 0xDx */
	{ "BNE",  _relative,      2, 2 },
	{ "CMP",  _zp_indirect_y, 2, 5, _Z | _N | _C },
	{ "CMP",  _zp_indirect,   2, 5, _Z | _N | _C },
	{ "",     _invalid,       1, 1 },
	{ "",     _invalid,       2, 4 },
	{ "CMP",  _zp_x,          2, 4, _Z | _N | _C },
	{ "DEC",  _zp_x,          2, 6, _Z | _N },
	{ "SMB5", _zp,            2, 5 },
	{ "CLD",  _implicit,      1, 2 },
	{ "CMP",  _absolute_y,    3, 4, _Z | _N | _C },
	{ "PHX",  _implicit,      1, 3 },
	{ "STP",  _implicit,      1, 2 },
	{ "",     _invalid,       3, 4 },
	{ "CMP",  _absolute_x,    3, 4, _Z | _N | _C },
	{ "DEC",  _absolute_x,    3, 7, _Z | _N },
	{ "BBS5", _zp_relative,   3, 4 },
	
	/* 0xEx */
	{ "CPX",  _immediate,     2, 2, _Z | _N | _C },
	{ "SBC",  _zp_indirect_x, 2, 6, _N | _Z | _C | _V },
	{ "",     _invalid,       2, 2 },
	{ "",     _invalid,       1, 1 },
	{ "CPX",  _zp,            2, 3, _Z | _N | _C },
	{ "SBC",  _zp,            2, 3, _N | _Z | _C | _V },
	{ "INC",  _zp,            2, 5, _Z | _N },
	{ "SMB6", _zp,            2, 5 },
	{ "INX",  _implicit,      1, 2, _Z | _N },
	{ "SBC",  _immediate,     2, 2, _N | _Z | _C | _V },
	{ "NOP",  _implicit,      1, 2 },
	{ "",     _invalid,       1, 1 },
	{ "CPX",  _absolute,      3, 4, _Z | _N | _C },
	{ "SBC",  _absolute,      3, 4, _N | _Z | _C | _V },
	{ "INC",  _absolute,      3, 6, _Z | _N },
	{ "BBS6", _zp_relative,   3, 4 },
	
	/* 0xFx */
	{ "BEQ",  _relative,      2, 2 },
	{ "SBC",  _zp_indirect_y, 2, 5, _N | _Z | _C | _V },
	{ "SBC",  _zp_indirect,   2, 5, _N | _Z | _C | _V },
	{ "",     _invalid,       1, 1 },
	{ "",     _invalid,       2, 4 },
	{ "SBC",  _zp_x,          2, 4, _N | _Z | _C | _V },
	{ "INC",  _zp_x,          2, 6, _Z | _N },
	{ "SMB7", _zp,            2, 5 },
	{ "SED",  _implicit,      1, 2 },
	{ "SBC",  _absolute_y,    3, 4, _N | _Z | _C | _V },
	{ "PLX",  _implicit,      1, 4, _Z | _N },
	{ "",     _invalid,       1, 1 },
	{ "",     _invalid,       3, 4 },
	{ "SBC",  _absolute_x,    3, 4, _N | _Z | _C | _V },
	{ "INC",  _absolute_x,    3, 7, _Z | _N },
	{ "BBS7", _zp_relative,   3, 4 },
};

static inline uint8_t _read_u8(struct cpu_65c02_t *s, uint16_t addr)
{
	return(s->mem->read(s->mem->private, addr));
}

static inline int8_t _read_i8(struct cpu_65c02_t *s, uint16_t addr)
{
	return((int8_t) s->mem->read(s->mem->private, addr));
}

static inline uint16_t _read_u16(struct cpu_65c02_t *s, uint16_t addr)
{
	return(
		s->mem->read(s->mem->private, addr) |
	       (s->mem->read(s->mem->private, addr + 1) << 8)
	);
}

static inline uint16_t _read_u16w(struct cpu_65c02_t *s, uint16_t addr)
{
	/* Wrap around inside page when reading MSB */
	return(
		s->mem->read(s->mem->private, addr) |
	       (s->mem->read(s->mem->private, (addr & 0xFF00) + ((addr + 1) & 0xFF)) << 8)
	);
}

static inline void _write_u8(struct cpu_65c02_t *s, uint16_t addr, uint8_t v)
{
	s->mem->write(s->mem->private, addr, v);
}

static inline void _push_u8(struct cpu_65c02_t *s, uint8_t v)
{
	_write_u8(s, 0x0100 + s->sp--, v);
}

static inline void _push_u16(struct cpu_65c02_t *s, uint16_t v)
{
	_push_u8(s, (v & 0xFF00) >> 8);
	_push_u8(s, (v & 0x00FF) >> 0);
}

static inline uint8_t _pull_u8(struct cpu_65c02_t *s)
{
	return(_read_u8(s, 0x0100 + ++s->sp));
}

static inline uint16_t _pull_u16(struct cpu_65c02_t *s)
{
	uint16_t v;
	v  = _pull_u8(s);
	v |= _pull_u8(s) << 8;
	return(v);
}

static inline uint8_t _cmp(struct cpu_65c02_t *s, uint8_t r, uint8_t m)
{
	s->c = (r >= m ? 1 : 0);
	return(r - m);
}

static inline uint8_t _adc(struct cpu_65c02_t *s, uint8_t m)
{
	uint16_t sum;
	
	if(s->d)
	{
		sum = (s->a & 0x0F) + (m & 0x0F) + s->c;
		if(sum >= 0x0A) sum += 0x06;
		
		sum += (s->a & 0xF0) + (m & 0xF0);
		if(sum >= 0xA0) sum += 0x60;
	}
	else
	{
		sum = s->a + m + s->c;
	}
	
	s->c = sum > 0xFF ? 1 : 0;
	s->v = (~(s->a ^ m) & (s->a ^ sum) & 0x80) ? 1 : 0;
	
	return(sum & 0xFF);
}

static inline uint8_t _sbc(struct cpu_65c02_t *s, uint8_t m)
{
	return(_adc(s, s->d ? 0x99 - m : ~m));
}

static inline uint8_t _pack_status(struct cpu_65c02_t *s)
{
	uint8_t v;
	
	v = (s->n ? _N : 0)
	  | (s->v ? _V : 0)
	  | (1 << 5)
	  | 0
	  | (s->d ? _D : 0)
	  | (s->i ? _I : 0)
	  | (s->z ? _Z : 0)
	  | (s->c ? _C : 0);
	
	return(v);
}

static inline void _unpack_status(struct cpu_65c02_t *s, uint8_t v)
{
	s->n = (v & _N ? 1 : 0);
	s->v = (v & _V ? 1 : 0);
	s->d = (v & _D ? 1 : 0);
	s->i = (v & _I ? 1 : 0);
	s->z = (v & _Z ? 1 : 0);
	s->c = (v & _C ? 1 : 0);
}

void cpu_65c02_init(struct cpu_65c02_t *s, int clock_num, int clock_den, struct cpu_memory_t *mem)
{
	memset(s, 0, sizeof(struct cpu_65c02_t));
	
	s->clock_num = clock_num;
	s->clock_den = clock_den;
	s->cycle = 0;
	s->verbose = 0;
	s->mem = mem;
	
	cpu_65c02_reset(s);
}

void cpu_65c02_reset(struct cpu_65c02_t *s)
{
	s->n = 0;
	s->v = 0;
	s->d = 0;
	s->i = 1;
	s->z = 1;
	s->c = 0;
	
	s->a = 0x00;
	s->x = 0x00;
	s->y = 0x00;
	s->sp = 0xFF;
	s->pc = _read_u16(s, 0xFFFC);
	
	s->depth = 0;
}

void cpu_65c02_irq_custom(struct cpu_65c02_t *s, uint16_t addr, int brk)
{
	_push_u16(s, s->pc);
	_push_u8(s, _pack_status(s) | (brk ? _B : 0));
	s->i = 1;
	s->d = 0; /* 65C02 only */
	s->pc = addr;
	s->depth++;
}

void cpu_65c02_irq(struct cpu_65c02_t *s, int nmi)
{
	/* Don't interrupt in inhibit flag is set, unless NMI */
	if(!nmi && s->i) return;
	cpu_65c02_irq_custom(s, _read_u16(s, nmi ? 0xFFFA : 0xFFFE), 0);
}

void cpu_65c02_exec(struct cpu_65c02_t *s)
{
	uint8_t op = _read_u8(s, s->pc);
	const struct _instr_t *ins = &_instrs[op];
	uint8_t r = 0;
	uint8_t tc;
	uint16_t m16 = 0x0000;
	uint8_t m8 = 0x00;
	int8_t m8b = 0x00;
	uint16_t addr = 0x0000;
	
	switch(ins->mode)
	{
	case _invalid:
	case _implicit:
	case _a:
		break;
	
	case _relative:
		m8 = _read_u8(s, s->pc + 1);
		addr = s->pc + ins->l + (int8_t) m8;
		break;
	
	case _zp:
		m8 = _read_u8(s, s->pc + 1);
		addr = m8;
		break;
	
	case _zp_x:
		m8 = _read_u8(s, s->pc + 1);
		addr = (m8 + s->x) & 0xFF;
		break;
	
	case _zp_y:
		m8 = _read_u8(s, s->pc + 1);
		addr = (m8 + s->y) & 0xFF;
		break;
	
	case _immediate:
		m8 = _read_u8(s, s->pc + 1);
		break;
	
	case _zp_indirect_x:
		m8 = _read_u8(s, s->pc + 1);
		addr = _read_u16w(s, (m8 + s->x) & 0xFF);
		break;
	
	case _zp_indirect_y:
		m8 = _read_u8(s, s->pc + 1);
		addr = _read_u16w(s, m8) + s->y;
		break;
	
	case _zp_indirect:
		m8 = _read_u8(s, s->pc + 1);
		addr = _read_u16w(s, m8);
		break;
	
	case _indirect:
		m16 = _read_u16(s, s->pc + 1);
		addr = _read_u16(s, m16);
		break;
	
	case _absolute:
		m16 = _read_u16(s, s->pc + 1);
		addr = m16;
		break;
	
	case _absolute_x:
		m16 = _read_u16(s, s->pc + 1);
		addr = m16 + s->x;
		break;
	
	case _absolute_y:
		m16 = _read_u16(s, s->pc + 1);
		addr = m16 + s->y;
		break;
	
	case _indirect_x:
		m16 = _read_u16(s, s->pc + 1);
		addr = _read_u16(s, m16 + s->x);
		break;
	
	case _zp_relative:
		m8 = _read_u8(s, s->pc + 1);
		m8b = _read_i8(s, s->pc + 2);
		addr = s->pc + ins->l + m8b;
		break;
	}
	
	if(s->verbose)
	{
		printf("[%lu PC:%04X A:%02X X:%02X Y:%02X SP:%02X %c%c--%c%c%c%c OP:%02X] %.*s%s",
			s->cycle,
			s->pc, s->a, s->x, s->y, s->sp,
			s->n ? 'N' : '.',
			s->v ? 'V' : '.',
			s->d ? 'D' : '.',
			s->i ? 'I' : '.',
			s->z ? 'Z' : '.',
			s->c ? 'C' : '.',
			op,
			s->depth,
			"                                     ",
			ins->m
		);
		
		switch(ins->mode)
		{
		case _invalid:       break;
		case _implicit:      break;
		case _a:             printf(" A"); break;
		case _immediate:     printf(" #$%02X", m8); break;
		case _absolute:      printf(" $%04X", addr); break;
		case _absolute_x:    printf(" $%04X,X ; == $%04X", m16, addr); break;
		case _absolute_y:    printf(" $%04X,Y ; == $%04X", m16, addr); break;
		case _relative:      printf(" $%04X", addr); break;
		case _zp:            printf(" $%02X", addr); break;
		case _zp_x:          printf(" $%02X,X ; == $%02X", m8, addr); break;
		case _zp_y:          printf(" $%02X,Y ; == $%02X", m8, addr); break;
		case _indirect:      printf(" ($%04X) ; == $%04X", m16, addr); break;
		case _zp_indirect:   printf(" ($%02X) ; == $%04X", m8, addr); break;
		case _zp_indirect_x: printf(" ($%02X,X) ; == $%04X", m8, addr); break;
		case _indirect_x:    printf(" ($%04X,X) ; == $%04X", m16, addr); break;
		case _zp_indirect_y: printf(" ($%02X),Y ; == $%04X", m8, addr); break;
		case _zp_relative:   printf(" $%02X,$%04X", m8, addr); break;
		}
		printf("          \n");
	}
	
	switch(op)
	{
	case 0x00: s->pc += ins->l; cpu_65c02_irq_custom(s, _read_u16(s, 0xFFFE) - ins->l, 1); break; /* BRK */
	case 0x01: r = s->a |= _read_u8(s, addr); break; /* ORA ($zp,x) */
	case 0x04: r = _read_u8(s, addr); _write_u8(s, addr, r | s->a); r &= s->a; break; /* TSB $zp */
	case 0x05: r = s->a |= _read_u8(s, addr); break; /* ORA $zp */
	case 0x06: r = _read_u8(s, addr); s->c = r >> 7; r <<= 1; _write_u8(s, addr, r); break; /* ASL $zp */
	case 0x07: r = _read_u8(s, addr) & 0xFE; _write_u8(s, addr, r); break; /* RMB0 $zp */
	case 0x08: _push_u8(s, _pack_status(s) | _B); break; /* PHP */
	case 0x09: r = s->a |= m8; break; /* ORA # */
	case 0x0A: s->c = s->a >> 7; r = s->a <<= 1; break; /* ASL A */
	case 0x0C: r = _read_u8(s, addr); _write_u8(s, addr, r | s->a); r &= s->a; break; /* TSB $addr */
	case 0x0D: r = s->a |= _read_u8(s, addr); break; /* ORA $addr */
	case 0x0E: r = _read_u8(s, addr); s->c = r >> 7; r <<= 1; _write_u8(s, addr, r); break; /* ASL $addr */
	case 0x0F: if((_read_u8(s, m8) & 0x01) == 0) s->pc += m8b; break; /* BBR0 $zp,$raddr */
	case 0x10: if(!s->n) s->pc += (int8_t) m8; break; /* BPL $raddr */
	case 0x11: r = s->a |= _read_u8(s, addr); break; /* ORA ($zp),y */
	case 0x12: r = s->a |= _read_u8(s, addr); break; /* ORA ($zp) */
	case 0x14: r = _read_u8(s, addr); _write_u8(s, addr, r & (0xFF ^ s->a)); r &= s->a; break; /* TRB $zp */
	case 0x15: r = s->a |= _read_u8(s, addr); break; /* ORA $zp,x */
	case 0x16: r = _read_u8(s, addr); s->c = r >> 7; r <<= 1; _write_u8(s, addr, r); break; /* ASL $zp,x */
	case 0x17: r = _read_u8(s, addr) & 0xFD; _write_u8(s, addr, r); break; /* RMB1 $zp */
	case 0x18: s->c = 0; break; /* CLC */
	case 0x19: r = s->a |= _read_u8(s, addr); break; /* ORA $addr,y */
	case 0x1A: r = ++s->a; break; /* INC A */
	case 0x1C: r = _read_u8(s, addr); _write_u8(s, addr, r & (0xFF ^ s->a)); r &= s->a; break; /* TRB $addr */
	case 0x1D: r = s->a |= _read_u8(s, addr); break; /* ORA $addr,x */
	case 0x1E: r = _read_u8(s, addr); s->c = r >> 7; r <<= 1; _write_u8(s, addr, r); break; /* ASL $addr,x */
	case 0x1F: if((_read_u8(s, m8) & 0x02) == 0) s->pc += m8b; break; /* BBR1 $zp,$raddr */
	case 0x20: _push_u16(s, s->pc + 2); s->pc = addr - ins->l; s->depth++; break; /* JSR $addr */
	case 0x21: r = s->a &= _read_u8(s, addr); break; /* AND ($zp,x) */
	case 0x24: r = _read_u8(s, addr); s->n = r >> 7; s->v = (r >> 6) & 1; r &= s->a; break; /* BIT $zp */
	case 0x25: r = s->a &= _read_u8(s, addr); break; /* AND $zp */
	case 0x26: r = _read_u8(s, addr); tc = s->c; s->c = r >> 7; r = (r << 1) | tc; _write_u8(s, addr, r); break; /* ROL $zp */
	case 0x27: r = _read_u8(s, addr) & 0xFB; _write_u8(s, addr, r); break; /* RMB2 $zp */
	case 0x28: _unpack_status(s, _pull_u8(s)); break; /* PLP */
	case 0x29: r = s->a &= m8; break; /* AND # */
	case 0x2A: tc = s->c; s->c = s->a >> 7; r = s->a = (s->a << 1) | tc; break; /* ROL A */
	case 0x2C: r = _read_u8(s, addr); s->n = r >> 7; s->v = (r >> 6) & 1; r &= s->a; break; /* BIT $addr */
	case 0x2D: r = s->a &= _read_u8(s, addr); break; /* AND $addr */
	case 0x2E: r = _read_u8(s, addr); tc = s->c; s->c = r >> 7; r = (r << 1) | tc; _write_u8(s, addr, r); break; /* ROL $addr */
	case 0x2F: if((_read_u8(s, m8) & 0x04) == 0) s->pc += m8b; break; /* BBR2 $zp,$raddr */
	case 0x30: if(s->n) s->pc += (int8_t) m8; break; /* BMI $raddr */
	case 0x31: r = s->a &= _read_u8(s, addr); break; /* AND ($zp),y */
	case 0x32: r = s->a &= _read_u8(s, addr); break; /* AND ($zp) */
	case 0x34: r = _read_u8(s, addr); s->n = r >> 7; s->v = (r >> 6) & 1; r &= s->a; break; /* BIT $zp,x */
	case 0x35: r = s->a &= _read_u8(s, addr); break; /* AND $zp,x */
	case 0x36: r = _read_u8(s, addr); tc = s->c; s->c = r >> 7; r = (r << 1) | tc; _write_u8(s, addr, r); break; /* ROL $zp,x */
	case 0x37: r = _read_u8(s, addr) & 0xF7; _write_u8(s, addr, r); break; /* RMB3 $zp */
	case 0x38: s->c = 1; break; /* SEC */
	case 0x39: r = s->a &= _read_u8(s, addr); break; /* AND $addr,y */
	case 0x3A: r = --s->a; break; /* DEC A */
	case 0x3C: r = _read_u8(s, addr); s->n = r >> 7; s->v = (r >> 6) & 1; r &= s->a; break; /* BIT $addr,x */
	case 0x3D: r = s->a &= _read_u8(s, addr); break; /* AND $addr,x */
	case 0x3E: r = _read_u8(s, addr); tc = s->c; s->c = r >> 7; r = (r << 1) | tc; _write_u8(s, addr, r); break; /* ROL $addr,x */
	case 0x3F: if((_read_u8(s, m8) & 0x08) == 0) s->pc += m8b; break; /* BBR3 $zp,$raddr */
	case 0x40: _unpack_status(s, _pull_u8(s)); s->pc = _pull_u16(s) - ins->l; s->depth--; break; /* RTI */
	case 0x41: r = s->a ^= _read_u8(s, addr); break; /* EOR ($zp,x) */
	case 0x45: r = s->a ^= _read_u8(s, addr); break; /* EOR $zp */
	case 0x46: r = _read_u8(s, addr); s->c = r & 1; r >>= 1; _write_u8(s, addr, r); break; /* LSR $zp */
	case 0x47: r = _read_u8(s, addr) & 0xEF; _write_u8(s, addr, r); break; /* RMB4 $zp */
	case 0x48: _push_u8(s, s->a); break; /* PHA */
	case 0x49: r = s->a ^= m8; break; /* EOR # */
	case 0x4A: s->c = s->a & 1; r = s->a >>= 1; break; /* LSR A */
	case 0x4C: s->pc = addr - ins->l; break; /* JMP $addr */
	case 0x4D: r = s->a ^= _read_u8(s, addr); break; /* EOR $addr */
	case 0x4E: r = _read_u8(s, addr); s->c = r & 1; r >>= 1; _write_u8(s, addr, r); break; /* LSR $addr */
	case 0x4F: if((_read_u8(s, m8) & 0x10) == 0) s->pc += m8b; break; /* BBR4 $zp,$raddr */
	case 0x50: if(!s->v) s->pc += (int8_t) m8; break; /* BVC $raddr */
	case 0x51: r = s->a ^= _read_u8(s, addr); break; /* EOR ($zp),y */
	case 0x52: r = s->a ^= _read_u8(s, addr); break; /* EOR ($zp) */
	case 0x55: r = s->a ^= _read_u8(s, addr); break; /* EOR $zp,x */
	case 0x56: r = _read_u8(s, addr); s->c = r & 1; r >>= 1; _write_u8(s, addr, r); break; /* LSR $zp,x */
	case 0x57: r = _read_u8(s, addr) & 0xDF; _write_u8(s, addr, r); break; /* RMB5 $zp */
	case 0x58: s->i = 0; break; /* CLI */
	case 0x59: r = s->a ^= _read_u8(s, addr); break; /* EOR $addr,y */
	case 0x5A: _push_u8(s, s->y); break; /* PHY */
	case 0x5D: r = s->a ^= _read_u8(s, addr); break; /* EOR $addr,x */
	case 0x5E: r = _read_u8(s, addr); s->c = r & 1; r >>= 1; _write_u8(s, addr, r); break; /* LSR $addr,x */
	case 0x5F: if((_read_u8(s, m8) & 0x20) == 0) s->pc += m8b; break; /* BBR5 $zp,$raddr */
	case 0x60: s->pc = _pull_u16(s); s->depth--; break; /* RTS */
	case 0x61: r = s->a = _adc(s, _read_u8(s, addr)); break; /* STA ($zp,x) */
	case 0x64: _write_u8(s, addr, 0); break; /* STZ $zp */
	case 0x65: r = s->a = _adc(s, _read_u8(s, addr)); break; /* ADC $zp */
	case 0x66: r = _read_u8(s, addr); tc = s->c << 7; s->c = r & 1; r = (r >> 1) | tc; _write_u8(s, addr, r); break; /* ROR $zp */
	case 0x67: r = _read_u8(s, addr) & 0xBF; _write_u8(s, addr, r); break; /* RMB6 $zp */
	case 0x68: r = s->a = _pull_u8(s); break; /* PLA */
	case 0x69: r = s->a = _adc(s, m8); break; /* ADC # */
	case 0x6A: tc = s->c << 7; s->c = s->a & 1; r = s->a = (s->a >> 1) | tc; break; /* ROR A */
	case 0x6C: s->pc = addr - ins->l; break; /* JMP ($addr) */
	case 0x6E: r = _read_u8(s, addr); tc = s->c << 7; s->c = r & 1; r = (r >> 1) | tc; _write_u8(s, addr, r); break; /* ROR $addr */
	case 0x6D: r = s->a = _adc(s, _read_u8(s, addr)); break; /* ADC $addr */
	case 0x6F: if((_read_u8(s, m8) & 0x40) == 0) s->pc += m8b; break; /* BBR6 $zp,$raddr */
	case 0x70: if(s->v) s->pc += (int8_t) m8; break; /* BVS $raddr */
	case 0x71: r = s->a = _adc(s, _read_u8(s, addr)); break; /* ADC ($zp),y */
	case 0x72: r = s->a = _adc(s, _read_u8(s, addr)); break; /* ADC ($zp) */
	case 0x74: _write_u8(s, addr, 0); break; /* STZ $zp,x */
	case 0x75: r = s->a = _adc(s, _read_u8(s, addr)); break; /* ADC $zp,x */
	case 0x76: r = _read_u8(s, addr); tc = s->c << 7; s->c = r & 1; r = (r >> 1) | tc; _write_u8(s, addr, r); break; /* ROR $zp,x */
	case 0x77: r = _read_u8(s, addr) & 0x7F; _write_u8(s, addr, r); break; /* RMB7 $zp */
	case 0x78: s->i = 1; break; /* SEI */
	case 0x79: r = s->a = _adc(s, _read_u8(s, addr)); break; /* ADC $addr,y */
	case 0x7A: r = s->y = _pull_u8(s); break; /* PLY */
	case 0x7C: s->pc = addr - ins->l; break; /* JMP ($addr,x) */
	case 0x7D: r = s->a = _adc(s, _read_u8(s, addr)); break; /* ADC $addr,x */
	case 0x7E: r = _read_u8(s, addr); tc = s->c << 7; s->c = r & 1; r = (r >> 1) | tc; _write_u8(s, addr, r); break; /* ROR $addr,x */
	case 0x7F: if((_read_u8(s, m8) & 0x80) == 0) s->pc += m8b; break; /* BBR7 $zp,$raddr */
	case 0x80: s->pc += (int8_t) m8; break; /* BRA $raddr */
	case 0x81: _write_u8(s, addr, s->a); break; /* STA ($zp,x) */
	case 0x84: _write_u8(s, addr, s->y); break; /* STY $zp */
	case 0x85: _write_u8(s, addr, s->a); break; /* STA $zp */
	case 0x86: _write_u8(s, addr, s->x); break; /* STX $zp */
	case 0x87: r = _read_u8(s, addr) | 0x01; _write_u8(s, addr, r); break; /* SMB0 $zp */
	case 0x88: r = --s->y; break; /* DEY */
	case 0x89: r = s->a & m8; break; /* BIT # */
	case 0x8A: r = s->a = s->x; break; /* TXA */
	case 0x8C: _write_u8(s, addr, s->y); break; /* STY $addr */
	case 0x8D: _write_u8(s, addr, s->a); break; /* STA $addr */
	case 0x8E: _write_u8(s, addr, s->x); break; /* STX $addr */
	case 0x8F: if(_read_u8(s, m8) & 0x01) s->pc += m8b; break; /* BBS0 $zp,$raddr */
	case 0x90: if(!s->c) s->pc += (int8_t) m8; break; /* BCC $raddr */
	case 0x91: _write_u8(s, addr, s->a); break; /* STA ($zp),y */
	case 0x92: _write_u8(s, addr, s->a); break; /* STA ($zp) */
	case 0x94: _write_u8(s, addr, s->y); break; /* STY $zp,x */
	case 0x95: _write_u8(s, addr, s->a); break; /* STA $zp,x */
	case 0x96: _write_u8(s, addr, s->x); break; /* STX $zp,y */
	case 0x97: r = _read_u8(s, addr) | 0x02; _write_u8(s, addr, r); break; /* SMB1 $zp */
	case 0x98: r = s->a = s->y; break; /* TYA */
	case 0x99: _write_u8(s, addr, s->a); break; /* STA $addr,y */
	case 0x9A: s->sp = s->x; break; /* TXS */
	case 0x9C: _write_u8(s, addr, 0); break; /* STZ $addr */
	case 0x9D: _write_u8(s, addr, s->a); break; /* STA $addr,x */
	case 0x9E: _write_u8(s, addr, 0); break; /* STZ $addr,x */
	case 0x9F: if(_read_u8(s, m8) & 0x02) s->pc += m8b; break; /* BBS1 $zp,$raddr */
	case 0xA0: r = s->y = m8; break; /* LDY # */
	case 0xA1: r = s->a = _read_u8(s, addr); break; /* LDA ($zp,x) */
	case 0xA2: r = s->x = m8; break; /* LDX # */
	case 0xA4: r = s->y = _read_u8(s, addr); break; /* LDY $zp */
	case 0xA5: r = s->a = _read_u8(s, addr); break; /* LDA $zp */
	case 0xA6: r = s->x = _read_u8(s, addr); break; /* LDX $zp */
	case 0xA7: r = _read_u8(s, addr) | 0x04; _write_u8(s, addr, r); break; /* SMB2 $zp */
	case 0xA8: r = s->y = s->a; break; /* TAY */
	case 0xA9: r = s->a = m8; break; /* LDA # */
	case 0xAA: r = s->x = s->a; break; /* TAX */
	case 0xAC: r = s->y = _read_u8(s, addr); break; /* LDY $addr */
	case 0xAD: r = s->a = _read_u8(s, addr); break; /* LDA $addr */
	case 0xAE: r = s->x = _read_u8(s, addr); break; /* LDX $addr */
	case 0xAF: if(_read_u8(s, m8) & 0x04) s->pc += m8b; break; /* BBS2 $zp,$raddr */
	case 0xB0: if(s->c) s->pc += (int8_t) m8; break; /* BCS $raddr */
	case 0xB1: r = s->a = _read_u8(s, addr); break; /* LDA ($zp),y */
	case 0xB2: r = s->a = _read_u8(s, addr); break; /* LDA ($zp) */
	case 0xB4: r = s->y = _read_u8(s, addr); break; /* LDY $zp,x */
	case 0xB5: r = s->a = _read_u8(s, addr); break; /* LDA $zp,x */
	case 0xB6: r = s->x = _read_u8(s, addr); break; /* LDX $zp,y */
	case 0xB7: r = _read_u8(s, addr) | 0x08; _write_u8(s, addr, r); break; /* SMB3 $zp */
	case 0xB8: s->v = 0; break; /* CLV */
	case 0xB9: r = s->a = _read_u8(s, addr); break; /* LDA $addr,y */
	case 0xBA: r = s->x = s->sp; break; /* TSX */
	case 0xBC: r = s->y = _read_u8(s, addr); break; /* LDY $addr,x */
	case 0xBD: r = s->a = _read_u8(s, addr); break; /* LDA $addr,x */
	case 0xBE: r = s->x = _read_u8(s, addr); break; /* LDX $addr,y */
	case 0xBF: if(_read_u8(s, m8) & 0x08) s->pc += m8b; break; /* BBS3 $zp,$raddr */
	case 0xC0: r = _cmp(s, s->y, m8); break; /* CPY # */
	case 0xC1: r = _cmp(s, s->a, _read_u8(s, addr)); break; /* CMP ($zp,x) */
	case 0xC4: r = _cmp(s, s->y, _read_u8(s, addr)); break; /* CPY $zp */
	case 0xC5: r = _cmp(s, s->a, _read_u8(s, addr)); break; /* CMP $zp */
	case 0xC6: r = _read_u8(s, addr) - 1; _write_u8(s, addr, r); break; /* DEC $zp */
	case 0xC7: r = _read_u8(s, addr) | 0x10; _write_u8(s, addr, r); break; /* SMB4 $zp */
	case 0xC8: r = ++s->y; break; /* INY */
	case 0xC9: r = _cmp(s, s->a, m8); break; /* CMP # */
	case 0xCA: r = --s->x; break; /* DEX */
	case 0xCC: r = _cmp(s, s->y, _read_u8(s, addr)); break; /* CPY $addr */
	case 0xCD: r = _cmp(s, s->a, _read_u8(s, addr)); break; /* CMP $addr */
	case 0xCE: r = _read_u8(s, addr) - 1; _write_u8(s, addr, r); break; /* DEC $addr */
	case 0xCF: if(_read_u8(s, m8) & 0x10) s->pc += m8b; break; /* BBS4 $zp,$raddr */
	case 0xD0: if(!s->z) s->pc += (int8_t) m8; break; /* BNE $raddr */
	case 0xD1: r = _cmp(s, s->a, _read_u8(s, addr)); break; /* CMP ($zp),y */
	case 0xD2: r = _cmp(s, s->a, _read_u8(s, addr)); break; /* CMP ($zp) */
	case 0xD5: r = _cmp(s, s->a, _read_u8(s, addr)); break; /* CMP $zp,x */
	case 0xD6: r = _read_u8(s, addr) - 1; _write_u8(s, addr, r); break; /* DEC $zp,x */
	case 0xD7: r = _read_u8(s, addr) | 0x20; _write_u8(s, addr, r); break; /* SMB5 $zp */
	case 0xD8: s->d = 0; break; /* CLD */
	case 0xD9: r = _cmp(s, s->a, _read_u8(s, addr)); break; /* CMP $addr,y */
	case 0xDA: _push_u8(s, s->x); break; /* PHX */
	case 0xDD: r = _cmp(s, s->a, _read_u8(s, addr)); break; /* CMP $addr,x */
	case 0xDE: r = _read_u8(s, addr) - 1; _write_u8(s, addr, r); break; /* DEC $addr,x */
	case 0xDF: if(_read_u8(s, m8) & 0x20) s->pc += m8b; break; /* BBS5 $zp,$raddr */
	case 0xE0: r = _cmp(s, s->x, m8); break; /* CPX # */
	case 0xE1: r = s->a = _sbc(s, _read_u8(s, addr)); break; /* SBC ($zp,x) */
	case 0xE4: r = _cmp(s, s->x, _read_u8(s, addr)); break; /* CPX $zp */
	case 0xE5: r = s->a = _sbc(s, _read_u8(s, addr)); break; /* SBC $zp */
	case 0xE6: r = _read_u8(s, addr) + 1; _write_u8(s, addr, r); break; /* INC $zp */
	case 0xE7: r = _read_u8(s, addr) | 0x40; _write_u8(s, addr, r); break; /* SMB6 $zp */
	case 0xE8: r = ++s->x; break; /* INX */
	case 0xE9: r = s->a = _sbc(s, m8); break; /* SBC # */
	case 0xEA: break; /* NOP */
	case 0xEC: r = _cmp(s, s->x, _read_u8(s, addr)); break; /* CPX $addr */
	case 0xED: r = s->a = _sbc(s, _read_u8(s, addr)); break; /* SBC $addr */
	case 0xEE: r = _read_u8(s, addr) + 1; _write_u8(s, addr, r); break; /* INC $addr */
	case 0xEF: if(_read_u8(s, m8) & 0x40) s->pc += m8b; break; /* BBS6 $zp,$raddr */
	case 0xF0: if(s->z) s->pc += (int8_t) m8; break; /* BEQ $raddr */
	case 0xF1: r = s->a = _sbc(s, _read_u8(s, addr)); break; /* SBC ($zp),y */
	case 0xF2: r = s->a = _sbc(s, _read_u8(s, addr)); break; /* SBC ($zp) */
	case 0xF5: r = s->a = _sbc(s, _read_u8(s, addr)); break; /* SBC $zp,x */
	case 0xF6: r = _read_u8(s, addr) + 1; _write_u8(s, addr, r); break; /* INC $zp,x */
	case 0xF7: r = _read_u8(s, addr) | 0x80; _write_u8(s, addr, r); break; /* SMB7 $zp */
	case 0xF8: s->d = 1; break; /* SED */
	case 0xF9: r = s->a = _sbc(s, _read_u8(s, addr)); break; /* SBC $addr,y */
	case 0xFA: r = s->x = _pull_u8(s); break; /* PLX */
	case 0xFD: r = s->a = _sbc(s, _read_u8(s, addr)); break; /* SBC $addr,x */
	case 0xFE: r = _read_u8(s, addr) + 1; _write_u8(s, addr, r); break; /* INC $addr,x */
	case 0xFF: if(_read_u8(s, m8) & 0x80) s->pc += m8b; break; /* BBS7 $zp,$raddr */
	default: printf("Unknown opcode:\n%04X: %02X\n", s->pc, op); break;
	}
	
	if(ins->flags)
	{
		if(ins->flags & _Z) s->z = r ? 0 : 1; /* Zero flag */
		if(ins->flags & _N) s->n = r >> 7;    /* Negative flag */
	}
	
	s->pc += ins->l;
	s->cycle += ins->mcycles;
}

