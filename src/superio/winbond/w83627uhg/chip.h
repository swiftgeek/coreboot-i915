/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2009 Dynon Avionics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 $
 */

#ifndef SUPERIO_WINBOND_W83627UHG_CHIP_H
#define SUPERIO_WINBOND_W83627UHG_CHIP_H

#include <pc80/keyboard.h>
#include <uart8250.h>

struct superio_winbond_w83627uhg_config {

	struct pc_keyboard keyboard;
};

#endif
