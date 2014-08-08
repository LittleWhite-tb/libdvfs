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

#include <stdbool.h>

#include "dvfs_unit.h"
#include "dvfs_core.h"


/**
 * @file dvfs_context.h
 *
 * Describes all the structures and functions related to the whole DVFS
 * context (the whole system). This is the entry point of the library.
 *
 * @sa dvfs_unit
 */


/**
 * DVFS Context. Entry point for the library. Stores all the references to the
 * DVFS units available on the system.
 *
 * @sa dvfs_unit()
 */
typedef struct {
   unsigned int nb_units;  //!< Number of DVFS units on the system
   dvfs_unit **units;      //!< DVFS units we are handling
} dvfs_ctx;

/**
 * Starts controlling DVFS on the system.
 *
 * @param ppCtx the new DVFS context used in the various functions.
 * @param seq Tells if the frequency transitions must be synchronized or not.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c ppCtx is NULL.
 *         \retval DVFS_ERROR_MEM_ALLOC_FAILED if memory allocation failed.
 *         \retval DVFS_ERROR_RELATED_CORE_UNAVAILABLE the related core information is not available
 *
 * @sa dvfs_stop()
 */
int dvfs_start(dvfs_ctx** ppCtx, bool seq);

/**
 * Frees the memory associated to a DVFS context and restores the DVFS control
 * to its state before calling dvfs_start.
 *
 * @param ctx the context
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c ctx is NULL.
 *
 * \sa dvfs_start()
 */
int dvfs_stop(dvfs_ctx *ctx);

/**
 * Returns true if one of the DVFS unit on the system allows TurboBoost.
 *
 * @return \retval 1 if TurboBoost is available on one of the DVFS unit.
 *         \retval 0 if TurboBoost is not available.
 *         \retval DVFS_ERROR_FILE_FAILED if the information could not be read (likely because /proc/cpuinfo is not available), or can't be opened.
 */
int dvfs_has_TB();

/**
 * Sets the provided governor on all the DVFS units.
 *
 * @param ctx The DVFS context as provided by dvfs_start
 * @param gov The new governor to set
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c ctx or gov are NULL.
 */
int dvfs_set_gov(const dvfs_ctx *ctx, const char *gov);

/**
 * Sets the given frequency on all the DVFS units. The effects are unknown if
 * the current governor is not "userspace".
 *
 * @param ctx The DVFS context as provided by dvfs_start()
 * @param freq The new frequency to set.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c ctx is NULL.
 */
int dvfs_set_freq(dvfs_ctx *ctx, unsigned int freq);

/**
 * Returns the dvfs_core structure associated to the given core id.
 *
 * @param ctx The DVFS context as provided by dvfs_start()
 * @param ppCore The dvfs_core structure associated to the core or NULL if the core id
 * is not found.
 * @param core_id The core id.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c ctx or \c ppCore are NULL.
 *         \retval DVFS_ERROR_INVALID_CORE_ID if the core ID is not valid.
 */
int dvfs_get_core(const dvfs_ctx *ctx, const dvfs_core **ppCore, unsigned int core_id);

/**
 * Returns the DVFS unit associated with the given core.
 *
 * @param ctx The DVFS context as provided by dvfs_start()
 * @param core The core structure.
 * @param ppUnit The DVFS unit in charge of this core.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c ctx or \c core or ppUnit are NULL.
 *         \retval DVFS_ERROR_CORE_UNIT_MISMATCH if the unit can not be find.
 */
int dvfs_get_unit(const dvfs_ctx *ctx, const dvfs_core *core, const dvfs_unit **ppUnit);
