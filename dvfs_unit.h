/*
 * libdvfs - A light library to set CPU governor and frequency
 * Copyright (C) 2013-2014 Universite de Versailles
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
 * @param ppUnit pointer where the DVFS Unit instance will be placed
 * @param nb_cores The number of cores the unit handles.
 * @param cores The array of cores to handle. The array itself is freed when
 * calling \c dvfs_unit_close().
 * @param unit_id the ID of this DVFS unit.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c ppUnit or \c cores are NULL.
 *         \retval DVFS_ERROR_MEM_ALLOC_FAILED if memory allocation failed.
 *
 * @sa dvfs_unit_close()
 * @sa dvfs_core
 */
int dvfs_unit_open(dvfs_unit** ppUnit, unsigned int nb_cores, dvfs_core **cores, unsigned int unit_id);

/**
 * Frees the memory associated to a DVFS unit and restore their DVFS state. You
 * are not supposed to directly call this function; use rather \c dvfs_stop().
 * It calls \see dvfs_core_close() on every \see dvfs_core avaiable in this unit.
 *
 * @param unit The DVFS unit to close.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c unit or cores are NULL.
 *
 * @sa dvfs_unit_open()
 */
int dvfs_unit_close(dvfs_unit *unit);

/**
 * Sets a governor on all the cores we are in charge of.
 *
 * @param unit The DVFS unit.
 * @param gov The governor to set.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c unit or \c gov are NULL.
 *
 */
int dvfs_unit_set_gov(const dvfs_unit *unit, const char *gov);

/**
 * Sets the given frequency on all the unit cores. The effect is unknown if the
 * current governor is not "userspace".
 *
 * @param unit The DVFS unit.
 * @param freq The frequency to set.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c unit is NULL.
 */
int dvfs_unit_set_freq(const dvfs_unit *unit, unsigned int freq);

/**
 * Gets te number of cores available in this DVFS unit.
 *
 * @param unit The DVFS unit.
 * @param pNbCores Will be filled with the number of cores.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c unit or \c ppCore are NULL.
 */
int dvfs_unit_get_nb_cores(const dvfs_unit* unit, unsigned int* pNbCores);

/**
 * Gets the core with the given id if it is part of this DVFS unit, or NULL
 * otherwise.
 *
 * @param unit The DVFS unit.
 * @param ppCore the \see dvfs_core identified by \c id
 * @param id The core id.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c unit or \c ppCore are NULL.
 *         \retval DVFS_ERROR_INVALID_CORE_ID the \c id did not match any core.
 */
int dvfs_unit_get_core(const dvfs_unit *unit, dvfs_core** ppCore, unsigned int id);

/**
 * Gets the frequency currently set for the current DVFS unit.
 *
 * @param unit The DVFS unit.
 * @param pFreq the frequency currently set for the current DVFS unit.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c unit or \c pFreq are NULL.
 */
int dvfs_unit_get_freq(const dvfs_unit *unit, unsigned int* pFreq);

/**
 * Returns the index of the considered DVFS unit as stored in the
 * corresponding DVFS context structure array
 *
 * @param unit The DVFS unit.
 * @param pID the index of this DVFS unit.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c unit or \c pID are NULL.
 */
int dvfs_unit_get_id(const dvfs_unit *unit, unsigned int *pID);
