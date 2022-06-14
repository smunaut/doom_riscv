/*
 * config.h
 *
 * Copyright (C) 2021 Sylvain Munaut
 * All rights reserved.
 *
 * LGPL v3+, see LICENSE.lgpl3
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#pragma once

#define VID_BASE	0x81000000
#define UART_BASE	0x82000000
#define LED_BASE	0x83000000
#define ESPLNK_BASE	0x84000000

#define VID_CTRL_BASE	(VID_BASE + 0x00000)
#define VID_PAL_BASE	(VID_BASE + 0x10000)
#define VID_FB_BASE	(VID_BASE + 0x20000)
