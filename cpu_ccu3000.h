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

#ifndef _CPU_CCU3000_H
#define _CPU_CCU3000_H

#include "cpu_65c02.h"

struct cpu_ccu3000_timer_t {
	uint8_t ctrl[3];
	uint16_t prescaler;
	uint16_t accu;
	uint16_t adder;
};

struct cpu_ccu3000_t {
	struct cpu_65c02_t core;
	struct cpu_memory_t mem;
	struct cpu_memory_t *ext;
	
	uint8_t *ram;
	
	int irq_enabled;
	
	struct cpu_ccu3000_timer_t timer[3];
	
	uint8_t p5_ddr;
	uint8_t p5_data;
	uint8_t p5_data_in;
	
	uint8_t p6_ddr;
	uint8_t p6_data;
	uint8_t p6_data_in;
	
	uint8_t p7_ddr;
	uint8_t p7_data;
	uint8_t p7_data_in;
	
	uint8_t p8_ddr;
	uint8_t p8_data;
	uint8_t p8_data_in;
};

extern void cpu_ccu3000_init(struct cpu_ccu3000_t *s, int clock_num, int clock_den, struct cpu_memory_t *mem);
extern void cpu_ccu3000_reset(struct cpu_ccu3000_t *s);
extern void cpu_ccu3000_irq_custom(struct cpu_ccu3000_t *s, uint16_t addr, int brk);
extern void cpu_ccu3000_irq(struct cpu_ccu3000_t *s, int type);
extern void cpu_ccu3000_exec(struct cpu_ccu3000_t *s);

#endif

