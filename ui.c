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

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <pthread.h>
#include "ui.h"

static void _render_ui(struct sdl_ui *ui)
{
	SDL_Rect drect, srect;
	int x, y, xo, yo;
	int i;
	
	/* Blue background for OSD (TODO: Use actual background attributes) */
	drect = (SDL_Rect) { 0, 0, 594, 540 };
	SDL_SetRenderDrawColor(ui->renderer, 0x00, 0x00, 0xFF, 0x00);
	SDL_RenderFillRect(ui->renderer, &drect);
	
	/* OSD offset */
	xo = 0;
	yo = 18;
	
	i = 0;
	for(y = 0; y < 16; y++)
	{
		i++;
		for(x = 1; x < 32; x++)
		{
			uint8_t c = ui->osd[i++];
			srect = (SDL_Rect) { (c & 0x0F) * 9, (c >> 4) * 15, 9, 15 };
			drect = (SDL_Rect) { xo + x * (9 * 2), yo + y * (15 * 2), 9 * 2, 15 * 2 };
			SDL_RenderCopy(ui->renderer, ui->charset, &srect, &drect);
		}
	}
	
	/* Dark grey background for control panel */
	drect = (SDL_Rect) { 0, 540, 594, 540 + 64 };
	SDL_SetRenderDrawColor(ui->renderer, 0x10, 0x10, 0x10, 0x00);
	SDL_RenderFillRect(ui->renderer, &drect);
	
	SDL_SetTextureColorMod(ui->led, 0xFF, 0xFF, 0xFF);
	for(i = 0; i < 4; i++)
	{
		srect = (SDL_Rect) { (8 + i) * 48, 0, 48, 64 };
		drect = (SDL_Rect) { i * 48, 540, srect.w, srect.h };
		
		/* Highlight pressed buttons */
		if(ui->buttons & (1 << i))
		{
			SDL_SetRenderDrawColor(ui->renderer, 0x30, 0x30, 0x30, 0x00);
		}
		else
		{
			SDL_SetRenderDrawColor(ui->renderer, 0x10, 0x10, 0x10, 0x00);
		}
		
		SDL_RenderFillRect(ui->renderer, &drect);
		SDL_RenderCopy(ui->renderer, ui->led, &srect, &drect);
	}
	
	for(i = 0; i < 8; i++)
	{
		srect = (SDL_Rect) { i * 48, 0, 48, 64 };
		drect = (SDL_Rect) { 594 - 48 * 2, 540, srect.w, srect.h };
		
		if(ui->msd & (1 << i)) SDL_SetTextureColorMod(ui->led, 0xFF, 0x00, 0x00);
		else SDL_SetTextureColorMod(ui->led, 0x20, 0x10, 0x10);
		
		SDL_RenderCopy(ui->renderer, ui->led, &srect, &drect);
		
		if(ui->lsd & (1 << i)) SDL_SetTextureColorMod(ui->led, 0xFF, 0x00, 0x00);
		else SDL_SetTextureColorMod(ui->led, 0x20, 0x10, 0x10);
		
		drect.x = 594 - 48;
		SDL_RenderCopy(ui->renderer, ui->led, &srect, &drect);
	}
	
	SDL_RenderPresent(ui->renderer);
}

static void *_thread(void *arg)
{
	struct sdl_ui *ui = arg;
	SDL_Event event;
	
	SDL_RenderClear(ui->renderer);
	
	while(!ui->done)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_KEYDOWN:
				
				switch(event.key.keysym.sym)
				{
				case SDLK_ESCAPE: ui->done = 1; break;
				case SDLK_q: ui->buttons |= (1 << 0); break;
				case SDLK_w: ui->buttons |= (1 << 1); break;
				case SDLK_e: ui->buttons |= (1 << 2); break;
				case SDLK_r: ui->buttons |= (1 << 3); break;
				}
				
				break;
			
			case SDL_KEYUP:
				
				switch(event.key.keysym.sym)
				{
				case SDLK_q: ui->buttons &= ~(1 << 0); break;
				case SDLK_w: ui->buttons &= ~(1 << 1); break;
				case SDLK_e: ui->buttons &= ~(1 << 2); break;
				case SDLK_r: ui->buttons &= ~(1 << 3); break;
				}
				
				break;
			
			case SDL_QUIT:
				ui->done = 1;
				break;
			}
		}
		
		_render_ui(ui);
		SDL_Delay(1000 / 25);
	}
	
	return(0);
}

int ui_start(struct sdl_ui *ui)
{
	int r;
	
	memset(ui, 0, sizeof(struct sdl_ui));
	
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);
	
	SDL_CreateWindowAndRenderer(594, 540 + 64, 0, &ui->window, &ui->renderer);
	ui->led = IMG_LoadTexture(ui->renderer, "7led.png");
	ui->charset = IMG_LoadTexture(ui->renderer, "charset.png");
	
	r = pthread_create(&ui->thread, NULL, &_thread, (void *) ui);
	if(r != 0)
	{
		fprintf(stderr, "Error starting the UI thread.\n");
		return(-1);
	}
	
	return(0);
}

int ui_end(struct sdl_ui *ui)
{
	ui->done = 1;
	
	pthread_join(ui->thread, NULL);
	
	SDL_DestroyTexture(ui->charset);
	SDL_DestroyTexture(ui->led);
	SDL_DestroyRenderer(ui->renderer);
	SDL_DestroyWindow(ui->window);
	
	IMG_Quit();
	SDL_Quit();
	
	return(0);
}

