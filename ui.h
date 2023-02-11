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

#ifndef _UI_H
#define _UI_H

struct sdl_ui {
	
	/* SDL bits */
	SDL_Renderer *renderer;
	SDL_Texture *led;
	SDL_Texture *charset;
	SDL_Window *window;
	
	/* OSD */
	uint8_t osd[512];
	
	/* Digits */
	uint8_t lsd;
	uint8_t msd;
	
	/* Buttons */
	uint8_t buttons;
	
	/* TODO: Remote control */
	
	/* Thread control */
	pthread_t thread;
	int done;
};

extern int ui_start(struct sdl_ui *ui);
extern int ui_end(struct sdl_ui *ui);

#endif

