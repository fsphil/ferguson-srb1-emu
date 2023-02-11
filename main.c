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
#include <SDL2/SDL.h>
#include "cpu_65c02.h"
#include "cpu_ccu3000.h"
#include "ui.h"

struct _srb1_system_t {
	struct cpu_memory_t mem;
	struct cpu_ccu3000_t ccu;
	uint8_t *rom;
};

struct _acm_system_t {
	struct cpu_65c02_t cpu;
	struct cpu_memory_t mem;
	uint8_t *ram;
	uint8_t *rom;
	uint8_t *osd;
	uint16_t osd_ptr;
};

static uint8_t _srb1_memory_read(void *private, uint16_t addr)
{
	struct _srb1_system_t *s = private;
	
	if(addr >= 0x8000)
	{
		return(s->rom[addr - 0x8000]);
	}
	
	printf("srb1: invalid read $%04X\n", addr);
	
	return(0xFF);
}

static void _srb1_memory_write(void *private, uint16_t addr, uint8_t v)
{
	/* Writing to ROM? */
	printf("srb1: invalid write $%04X = $%02X\n", addr, v);
}

static int _srb1_memory_init(struct _srb1_system_t *s)
{
	FILE *f;
	
	s->rom = malloc(0x8000);
	if(!s->rom)
	{
		return(-1);
	}
	
	/* Read the ROM */
	f = fopen("firmware-srb1.bin", "rb");
	fread(s->rom, 1, 0x8000, f);
	fclose(f);
	
	s->mem.private = s;
	s->mem.read = &_srb1_memory_read;
	s->mem.write = &_srb1_memory_write;
	
	return(0);
}

static uint8_t _acm_memory_read(void *private, uint16_t addr)
{
	struct _acm_system_t *s = private;
	
	if(addr < 0x2000)
	{
		return(s->ram[addr]);
	}
	else if(addr < 0x8000)
	{
		//printf("acm: IO read: %04X\n", addr);
		return(0xFF);
	}
	
	return(s->rom[addr - 0x8000]);
}

static void _acm_memory_write(void *private, uint16_t addr, uint8_t v)
{
	struct _acm_system_t *s = private;
	
	if(addr < 0x2000)
	{
		s->ram[addr] = v;
	}
	else if(addr < 0x8000)
	{
		//printf("acm: IO write: %04X = %02X\n", addr, v);
		
		/* Writing to OSD */
		if(addr >= 0x4000 && addr <= 0x4003 && s->cpu.verbose)
		{
			printf("acm: OSD write: %04X = %02X\n", addr, v);
		}
		
		if(addr == 0x4000)
		{
			s->osd_ptr = (s->osd_ptr & 0xFF00) + v;
		}
		else if(addr == 0x4001)
		{
			s->osd_ptr = (s->osd_ptr & 0x00FF) + (v << 8);
		}
		else if(addr == 0x4002 && s->osd)
		{
			s->osd[s->osd_ptr++ & 0x1FF] = v;
		}
	}
	else
	{
		/* Writing to ROM? */
		//printf("acm: invalid write $%04X = $%02X\n", addr, v);
	}
}

static int _acm_memory_init(struct _acm_system_t *s)
{
	FILE *f;
	
	s->rom = malloc(0x8000);
	if(!s->rom)
	{
		return(-1);
	}
	
	/* Read the ROM */
	f = fopen("firmware-acm.bin", "rb");
	fread(s->rom, 1, 0x8000, f);
	fclose(f);
	
	/* Allocate the RAM */
	s->ram = malloc(0x2000);
	if(!s->ram)
	{
		return(-1);
	}
	
	/* Read the optional RAM image */
	f = fopen("bbram-acm.bin", "rb");
	if(f)
	{
		fread(s->ram, 1, 0x2000, f);
		fclose(f);
	}
	
	/* Fill the OSD with 'A' for test */
	s->osd_ptr = 0;
	
	s->mem.private = s;
	s->mem.read = &_acm_memory_read;
	s->mem.write = &_acm_memory_write;
	
	return(0);
}

int main(int argc, char *argv[])
{
	struct _srb1_system_t srb1;
	struct _acm_system_t acm;
	struct sdl_ui ui;
	int counter = 0;
	
	ui_start(&ui);
	
	/* Configure SRB1 system (4 MHz clock) */
	_srb1_memory_init(&srb1);
	cpu_ccu3000_init(&srb1.ccu, 4000000, 1, &srb1.mem);
	srb1.ccu.p5_data_in = 0xFF;
	srb1.ccu.p6_data_in = 0xFF;
	srb1.ccu.p8_data_in = 0xFF;
	srb1.ccu.core.verbose = 0;
	
	/* Configure ACM system (8 MHz clock - it's not) */
	_acm_memory_init(&acm);
	cpu_65c02_init(&acm.cpu, 8000000, 1, &acm.mem);
	acm.osd = ui.osd;
	acm.cpu.verbose = 0;
	
	/* Force a screen to be displayed on the OSD -- V1.50 ACM */
	//acm.cpu.pc = 0xD051; // Blank screen ($00, $20 ...)
	//acm.cpu.pc = 0xD048; // Transparent screen ($81, $00 ...)
	//acm.cpu.pc = 0xD26A; // Blank screen ($00, $20 ... same as D051?)
	//acm.cpu.pc = 0xD8E8; // Blank screen ($80 on top line, $81 on other, $00 ...)
	//acm.cpu.pc = 0xD002; // "HELP". Shows parental control number.
	//acm.cpu.pc = 0xC640; // "PROGRAM CONTROL"
	//acm.cpu.pc = 0xC69B; // "EQUIPMENT AUTH NUMBER" screen
	//acm.cpu.pc = 0xC795; // "PARENTAL CONTROL"
	//acm.cpu.pc = 0xCA33; // "PARENTAL CONTROL NUMBER"
	//acm.cpu.pc = 0xCA43; // "PAY-TV NUMBER"
	//acm.cpu.pc = 0xC18D; // "DIAGNOSTIC DATA" incomplete (requires ACM bus link?)
	//acm.cpu.pc = 0xCB5E; // "PAY-TV HISTORY"
	//acm.cpu.pc = 0xCDF0; // "PERSONAL MESSAGES"
	
	while(!ui.done)
	{
		cpu_ccu3000_exec(&srb1.ccu);
		cpu_65c02_exec(&acm.cpu);
		
		/* Update the LED display */
		/* The LEDs are illuminated if pin is output 1, or input */
		if((~srb1.ccu.p8_data | srb1.ccu.p8_ddr) & (1 << 3))
		{
			ui.lsd = ~srb1.ccu.p6_data | srb1.ccu.p6_ddr;
		}
		else
		{
			//ui.lsd = 0;
		}
		
		if((~srb1.ccu.p8_data | srb1.ccu.p8_ddr) & (1 << 2))
		{
			ui.msd = ~srb1.ccu.p6_data | srb1.ccu.p6_ddr;
		}
		else
		{
			//ui.msd = 0;
		}
		
		/* Update the buttons (pressed = 0) */
		srb1.ccu.p6_data_in = ~ui.buttons;
		
		if(srb1.ccu.core.cycle >= 794930)
		{
			if(counter % 4000 == 1000 - 1)
			{
				/* Trigger a Timer1 interrupt */
				//printf("* Timer1 interrupt\n");
				cpu_ccu3000_irq_custom(&srb1.ccu, _srb1_memory_read(&srb1, 0xFFF6) | (_srb1_memory_read(&srb1, 0xFFF7) << 8), 0);
			}
			else if(counter % 4000 == 2000 - 1)
			{
				/* Trigger a Timer2 interrupt */
				//printf("* Timer2 interrupt\n");
				cpu_ccu3000_irq_custom(&srb1.ccu, _srb1_memory_read(&srb1, 0xFFF4) | (_srb1_memory_read(&srb1, 0xFFF5) << 8), 0);
			}
			//else if(counter % 4000 == 3000 - 1)
			//{
			//	/* Trigger a Timer3 interrupt */
			//	printf("* Timer3 interrupt\n");
			//	cpu_ccu3000_irq_custom(&srb1.ccu, _srb1_memory_read(&srb1, 0xFFF2) | (_srb1_memory_read(&srb1, 0xFFF3) << 8), 0);
			//}
		}
		
		counter++;
	}
	
	ui_end(&ui);
	
	return(0);
}

