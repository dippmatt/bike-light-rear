/*
 * Copyright (c) 2025 Matthias Dippold
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * NON-COMMERCIAL USE CLAUSE:
 * Permission is granted to use, copy, modify, and distribute this software
 * and its derivatives for NON-COMMERCIAL purposes only. Commercial use of
 * this software, in whole or in part, is strictly prohibited without
 * explicit written permission from the copyright holder.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <zephyr/kernel.h>

/**
 * @brief Debug print macro that conditionally uses printk
 *
 * When DEBUG is enabled, this macro behaves like printk().
 * When DEBUG is disabled, this macro does nothing (no code generated).
 *
 * Usage: debug_printk("format string", args...)
 */
#ifdef DEBUG
#define debug_printk(...) printk(__VA_ARGS__)
#else
#define debug_printk(...) ((void)0)
#endif

#endif /* DEBUG_H */
