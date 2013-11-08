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

#pragma once

#include "core.h"

#include <stdbool.h>

/**
 * A DVFS unit. In order to understand what it is, the reader must understand
 * that several CPU models do not allow one to set different frequencies for the
 * CPU cores. Thus, the cores are groupped in DVFS units for which a single
 * frequency can be applied.
 *
 * \sa dvfs_core
 */
typedef struct {
   unsigned int nb_cores;     //<! number of cores in the unit
   dvfs_core **cores;  //<! cores in the unit
} dvfs_unit;

/**
 * Creates a new DVFS unit in charge of the provided cores. You are not supposed
 * to directly call this function, use rather \c dvfs_start().
 *
 * @param nb_cores The number of cores the unit handles.
 * @param cores The array of cores to handle. The array itself is freed when
 * calling \c dvfs_unit_close().
 *
 * @return A new DVFS unit or NULL in case of error.
 *
 * \sa dvfs_unit_close()
 * \sa dvfs_core()
 */
dvfs_unit *dvfs_unit_open(unsigned int nb_cores, dvfs_core **cores);

/**
 * Frees the memory associated to a DVFS unit and restore their DVFS state. You
 * are not supposed to directly call this function; use rather \c dvfs_stop().
 *
 * @param unit The DVFS unit to close.
 *
 * \sa dvfs_unit_open()
 */
void dvfs_unit_close(dvfs_unit *unit);

/**
 * Sets a governor on all the cores we are in charge of.
 *
 * @param unit The DVFS unit.
 * @param gov The governor to set.
 */
void dvfs_unit_set_gov(const dvfs_unit *unit, const char *gov);

/**
 * Sets the given frequency on all the unit cores. The effect is unknown if the
 * current governor is not "userspace".
 *
 * @param unit The DVFS unit.
 * @param freq The frequency to set.
 */
void dvfs_unit_set_freq(dvfs_unit *unit, unsigned int freq);

/**
 * Returns true if the the core with the given id is inside the given DVFS unit.
 *
 * @param unit The DVFS unit.
 * @param id The core id.
 *
 * @return True if the core with the given id is within \p unit.
 */
bool dvfs_unit_contains(dvfs_unit *unit, unsigned int id);

