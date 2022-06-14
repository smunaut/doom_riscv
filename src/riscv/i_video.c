/*
 * i_video.c
 *
 * Video system support code
 *
 * Copyright (C) 2021 Sylvain Munaut
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdint.h>
#include <string.h>

#include "doomdef.h"

#include "i_system.h"
#include "v_video.h"
#include "i_video.h"

#include "config.h"

static volatile struct {
	uint32_t csr;
	uint32_t lcd;
} * const video_regs = (void*)(VID_CTRL_BASE);

#define LCD_RST	(1 << 18)
#define LCD_CS	(1 << 17)


static const uint8_t lcd_init_data[] = {
	 3, 0xef, 0x03, 0x80, 0x02,		// ? (undocumented cmd)
	 3, 0xcf, 0x00, 0xc1, 0x30,		// Power control B
	 4, 0xed, 0x64, 0x03, 0x12, 0x81,	// Power on sequence control
	 3, 0xe8, 0x85, 0x00, 0x78,		// Driver timing control A
	 5, 0xcb, 0x39, 0x2c, 0x00, 0x34, 0x02,	// Power control A
	 1, 0xf7, 0x20,				// Pump ratio control
	 2, 0xea, 0x00, 0x00,			// Driver timing control B
	 1, 0xc0, 0x23,				// Power control 1
	 1, 0xc1, 0x10,				// Power control 2
	 2, 0xc5, 0x3e, 0x28,			// VCOM Control 1
	 1, 0xc7, 0x86,				// VCOM Control 2
	 1, 0x3a, 0x55,				// Pixel Format: 16b
	 3, 0xb6, 0x08, 0x82, 0x27,		// Display Function Control
	 1, 0xf2, 0x00,				// 3 Gamma control disable
	 1, 0x26, 0x01,				// Gamma Set
	15, 0xe0, 0x0f, 0x31, 0x2b, 0x0c, 0x0e,	// Positive Gamma Correction
	          0x08, 0x4e, 0xf1, 0x37, 0x07,
	          0x10, 0x03, 0x0e, 0x09, 0x00,
	15, 0xe1, 0x00, 0x0e, 0x14, 0x03, 0x11,	// Negative Gamma Correction
	          0x07, 0x31, 0xc1, 0x48, 0x08,
	          0x0f, 0x0c, 0x31, 0x36, 0x0f,
	 0, 0x11,				// Sleep Out
	 0, 0x29,				// Display ON
	 1, 0x35, 0x00,				// Tearing Effect Line ON
	 1, 0x36, 0x08,				// Memory Access Control
	 4, 0x2a, 0x00, 0x00, 0x00, 0xef,	// Column Address Set
	 4, 0x2b, 0x00, 0x00, 0x01, 0x3f,	// Page Address Set
};


static void
delay(int n)
{
	while (n--)
		for (int i=0; i<6250; i++)
			asm volatile ("nop");
}


void
I_InitGraphics(void)
{
	/* Reset the LCD */
	video_regs->csr = 0;
	delay(1);
	video_regs->csr = LCD_RST;
	delay(1);
	video_regs->csr = 0;
	delay(100);
	video_regs->csr = LCD_CS;
	delay(1);

	/* Play init sequence */
	for (int i=0; i<sizeof(lcd_init_data); ) {
		video_regs->lcd = lcd_init_data[i+1];
		for (int j=0; j<lcd_init_data[i]; j++)
			video_regs->lcd = lcd_init_data[i+2+j] | (1 << 8);
		i += 1 + 1 + lcd_init_data[i];
	}

	/* Trigger refresh */
	video_regs->lcd = 0x2c | (1 << 31);

	/* Set base */
#define DIRECT
#ifdef DIRECT
	screens[0] = (void*)VID_FB_BASE;
#endif

	/* Set gamma default */
	usegamma = 1;
}

void
I_ShutdownGraphics(void)
{
	/* Don't need to do anything really ... */
}


void
I_SetPalette(byte* palette)
{
	static volatile uint32_t * const video_pal = (void*)(VID_PAL_BASE);
	byte r, g, b;

	for (int i=0 ; i<256 ; i++) {
		r = gammatable[usegamma][*palette++];
		g = gammatable[usegamma][*palette++];
		b = gammatable[usegamma][*palette++];
		video_pal[i] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
	}
}


void
I_UpdateNoBlit(void)
{
}

void
I_FinishUpdate (void)
{
	/* Copy from RAM buffer to frame buffer */
#ifndef DIRECT
	memcpy(
		(void*)VID_FB_BASE,
		screens[0],
		SCREENHEIGHT * SCREENWIDTH
	);
#endif

	/* Trigger refresh */
	video_regs->lcd = 0x2c | (1 << 31) | (1 << 30);
#ifdef DIRECT
	while (video_regs->lcd & 0xf0000000);
#endif

	/* Very crude FPS measure (time to render 100 frames */
#if 1
	static int frame_cnt = 0;
	static int tick_prev = 0;

	if (++frame_cnt == 100)
	{
		int tick_now = I_GetTime();
		printf("%d\n", tick_now - tick_prev);
		tick_prev = tick_now;
		frame_cnt = 0;
	}
#endif
}


void
I_WaitVBL(int count)
{
	/* Buys-Wait for VBL status bit */
	video_regs->csr |= (1 << 16);
	while ((video_regs->csr & (1<<16)));
}


void
I_ReadScreen(byte* scr)
{
	/* FIXME: Would have though reading from VID_FB_BASE be better ...
	 *        but it seems buggy. Not sure if the problem is in the
	 *        gateware
	 */
	memcpy(
		scr,
		screens[0],
		SCREENHEIGHT * SCREENWIDTH
	);
}


#if 0	/* WTF ? Not used ... */
void
I_BeginRead(void)
{
}

void
I_EndRead(void)
{
}
#endif
