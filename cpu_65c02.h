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

#ifndef _CPU_65C02_H
#define _CPU_65C02_H

#include <stdint.h>

struct cpu_memory_t {
	uint8_t (*read) (void *private, uint16_t addr);
	void (*write) (void *private, uint16_t addr, uint8_t v);
	void *private;
};

struct cpu_65c02_t {
	
	int clock_num;
	int clock_den;
	
	uint64_t cycle;
	
	uint16_t pc; /* Program Counter */
	uint8_t  a;  /* Accumulator     */
	uint8_t  x;  /* X Register      */
	uint8_t  y;  /* Y Register      */
	uint8_t  sp; /* Stack Pointer   */
	
	/* Status register, expanded */
	uint8_t n;	/* Negative */
	uint8_t v;	/* Overflow */
	uint8_t d;	/* Decimal */
	uint8_t i;	/* Interrupt */
	uint8_t z;	/* Zero */
	uint8_t c;	/* Carry */
	
	/* JSR depth, for pretty formatting */
	uint8_t depth;
	
	/* Print lots of data to stdout */
	int verbose;
	
	/* Memory access */
	struct cpu_memory_t *mem;
};

extern void cpu_65c02_init(struct cpu_65c02_t *s, int clock_num, int clock_den, struct cpu_memory_t *mem);
extern void cpu_65c02_reset(struct cpu_65c02_t *s);
extern void cpu_65c02_irq_custom(struct cpu_65c02_t *s, uint16_t addr, int brk);
extern void cpu_65c02_irq(struct cpu_65c02_t *s, int type);
extern void cpu_65c02_exec(struct cpu_65c02_t *s);

#endif

