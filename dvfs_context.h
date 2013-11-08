/*
 * libdvfs - A light library to set CPU governor and frequency
 * Copyright (C) 2013 Universite de Versailles
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include "dvfs_unit.h"

#pragma once

/**
 * DVFS Context. Entry point for the library. Stores all the references to the 
 * DVFS units available on the system.
 * 
 * \sa dvfs_unit()
 */
typedef struct {
   unsigned int nb_units;  //!< Number of DVFS units on the system
   dvfs_unit **units;      //!< DVFS units we are handling
} dvfs_ctx;

/**
 * Starts controlling DVFS on the system.
 *
 * @return A new DVFS context used in the various functions or NULL in case of
 * error.
 *
 * \sa dvfs_stop()
 */
dvfs_ctx *dvfs_start();

/**
 * Frees the memory associated to a DVFS context and restores the DVFS control
 * to its state before calling dvfs_start.
 *
 * \sa dvfs_start()
 */
void dvfs_stop(dvfs_ctx *ctx);

/**
 * Returns true if one of the DVFS unit on the system allows TurboBoost.
 *
 * @return True if TurboBoost is available on one of the DVFS unit.
 */
bool dvfs_has_TB();

/**
 * Sets the provided governor on all the DVFS units.
 *
 * @param dvfs The DVFS context as provided by dvfs_start
 * @param gov The new governor to set
 */
void dvfs_set_gov(const dvfs_ctx *dvfs, const char *gov);

/**
 * Sets the given frequency on all the DVFS units. The effects are unknown if
 * the current governor is not "userspace".
 *
 * @param dvfs The DVFS context as provided by dvfs_start
 * @param freq The new frequency to set.
 */
void dvfs_set_freq(dvfs_ctx *ctx, unsigned int freq);
