/*
 * console.c
 *
 * Copyright (C) 2019-2021 Sylvain Munaut
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

#include "config.h"
#include "mini-printf.h"


struct wb_uart {
	uint32_t data;
	uint32_t clkdiv;
} __attribute__((packed,aligned(4)));

static volatile struct wb_uart * const uart_regs = (void*)(UART_BASE);


void
console_init(void)
{
	uart_regs->clkdiv = 23;	/* 1 Mbaud with clk=25MHz */
}

void
console_putchar(char c)
{
	uart_regs->data = c;
}

char
console_getchar(void)
{
	int32_t c;
	do {
		c = uart_regs->data;
	} while (c & 0x80000000);
	return c;
}

int
console_getchar_nowait(void)
{
	int32_t c;
	c = uart_regs->data;
	return c & 0x80000000 ? -1 : (c & 0xff);
}

void
console_puts(const char *p)
{
	char c;
	while ((c = *(p++)) != 0x00)
		uart_regs->data = c;
}

int
console_printf(const char *fmt, ...)
{
	static char _printf_buf[128];
        va_list va;
        int l;

        va_start(va, fmt);
        l = mini_vsnprintf(_printf_buf, 128, fmt, va);
        va_end(va);

	console_puts(_printf_buf);

	return l;
}
