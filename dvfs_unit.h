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

#include "dvfs_core.h"

/**
 * @file dvfs_unit.h
 * 
 * Structures and functions related to DVFS units (group of cores running
 * at the same frequency).
 *
 * @sa dvfs_core
 */

/**
 * A DVFS unit. In order to understand what it is, the reader must understand
 * that several CPU models do not allow one to set different frequencies for the
 * CPU cores. Thus, the cores are groupped in DVFS units for which a single
 * frequency can be applied.
 *
 * @sa dvfs_core()
 */
typedef struct {
   unsigned int id;      //!< Unit id as described in the dvfs_context structure
   unsigned int nb_cores;     //!< Number of cores in the unit
   dvfs_core **cores;         //!< Cores in the unit
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
 * @sa dvfs_unit_close()
 * @sa dvfs_core
 */
dvfs_unit *dvfs_unit_open(unsigned int nb_cores, dvfs_core **cores, unsigned int unit_id);

/**
 * Frees the memory associated to a DVFS unit and restore their DVFS state. You
 * are not supposed to directly call this function; use rather \c dvfs_stop().
 *
 * @param unit The DVFS unit to close.
 *
 * @sa dvfs_unit_open()
 */
void dvfs_unit_close(dvfs_unit *unit);

/**
 * Sets a governor on all the cores we are in charge of.
 *
 * @param unit The DVFS unit.
 * @param gov The governor to set.
 *
 * @return Upon successful completion 1 is return. Otherwise, 0 is return and errno is set appropriately.
 */
unsigned int dvfs_unit_set_gov(const dvfs_unit *unit, const char *gov);

/**
 * Sets the given frequency on all the unit cores. The effect is unknown if the
 * current governor is not "userspace".
 *
 * @param unit The DVFS unit.
 * @param freq The frequency to set.
 *
 * @return Upon successful completion 1 is return. Otherwise, 0 is return and errno is set appropriately.
 */
unsigned int dvfs_unit_set_freq(const dvfs_unit *unit, unsigned int freq);

/**
 * Returns the core with the given id if it is part of this DVFS unit, or NULL
 * otherwise.
 *
 * @param unit The DVFS unit.
 * @param id The core id.
 *
 * @return The core with the given id or NULL if the core is not within \p unit.
 */
const dvfs_core *dvfs_unit_get_core(const dvfs_unit *unit, unsigned int id);

/**
 * Returns the frequency currently set for the current DVFS unit.
 *
 * @param unit The DVFS unit.
 *
 * @return The frequency currently set for the whole unit.
 */
unsigned int dvfs_unit_get_freq(const dvfs_unit *unit);

/**
 * Returns the index of the considered DVFS unit as stored in the
 * corresponding DVFS context structure array
 */
unsigned int dvfs_unit_get_id (const dvfs_unit *unit);
