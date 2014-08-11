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

#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * @file dvfs_core.h
 *
 * Structures and functions to change frequency at the scale of a CPU
 * core.
 */

/**
 * Represents on core. A core allows to control CPU Core governor and frequency.
 */
typedef struct {
   unsigned int id;        //!< Core id as declared by Linux
   unsigned int nb_freqs;  //!< Number of frequencies available for this core
   unsigned int *freqs;    //!< Available frequencies for this core, sorted by increasing order

   FILE *fd_setf;          //!< File descriptor toward the \c set_speed file
   FILE *fd_getf;          //!< Descriptor toward the \c cur_freq file

   char init_gov[128];     //!< Governor used when core get initialised
   unsigned int init_freq; //!< Freqency used when core get initialised

   sem_t *sem;             //!< Semaphore for sequentialization. Can be NULL.
} dvfs_core;

/**
 * Opens the Core context for the given core ID.
 *
 * The dvfs_core is valid even if the semaphore failed. The frequency transitions will
 * not be seqeuntialized.
 *
 * @param ppCore the instanciated Core context for this core. May return NULL in case
 * of error. The error cases are often related to file opening (such as permission
 * denied). In such cases errno is set appropriately.
 * @param id The id of the core to control.
 * @param seq True when the frequency transitions must be sequentialized.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c ppUnit or \c cores are NULL.
 *         \retval DVFS_ERROR_MEM_ALLOC_FAILED if memory allocation failed.
 *
 * @sa dvfs_core_close()
 */
int dvfs_core_open(dvfs_core** ppCore, unsigned int id, bool seq);

/**
 * Closes properly an opened Core context.
 * Sets back the governor that was in place when opening the context.
 *
 * @param core The core to close.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c core is NULL.
 */
int dvfs_core_close(dvfs_core *core);

/**
 * Sets the current DVFS governor on the given core to the given buffer.
 *
 * @param core The core on which the governor has to be set.
 * @param buf The pointer to the buffer which will be set to the governor string value
 * @param buf_len The size of the buffer
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c core or \c buf are NULL.
 *         \retval DVFS_ERROR_BUFFER_TOO_SHORT if the buffer for the path to the governor file is too short
 *         \retval DVFS_ERROR_FILE_ERROR operation on file failed (you can check errno for more details).
 */
int dvfs_core_get_gov(const dvfs_core *core, char *buf, size_t buf_len);

/**
 * Changes the governor on the given core.
 *
 * @param core The core on which the governor has to be set.
 * @param gov The governor to set.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c core or \c buf are NULL.
 *         \retval DVFS_ERROR_BUFFER_TOO_SHORT if the buffer for the path to the governor file is too short.
 *         \retval DVFS_ERROR_FILE_ERROR operation on file failed (you can check errno for more details).
 */
int dvfs_core_set_gov(const dvfs_core *core, const char *gov);

/**
 * Sets the frequency for the given core. Assumes that the "userspace" governor
 * has been set, result is unknown otherwise.
 *
 * @param core The related core.
 * @param freq The frequency to set.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c core or \c buf are NULL.
 *         \retval DVFS_ERROR_SET_FREQ_FILE frequency file not operational.
 *         \retval DVFS_ERROR_FILE_ERROR operation on file failed (you can check errno for more details).
 */
int dvfs_core_set_freq(const dvfs_core *core, unsigned int freq);

/**
 * Gets the frequency currently set for the core. Warning, this is not
 * necessarily the frequency currently active for the core as other cores in the
 * same unit may have requested a different frequency. In order to determine the
 * frequency actually set for the core, use instead \p dvfs_unit_get_freq().
 *
 * @param core The CPU core.
 * @param pFreq The frequency currently set.
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c core or \c buf are NULL.
 *         \retval DVFS_ERROR_FILE_ERROR operation on file failed (you can check errno for more details).
 *
 * @sa dvfs_unit_get_freq()
 */
int dvfs_core_get_current_freq(const dvfs_core *core, unsigned int* pFreq);

/**
 * Gets the frequency currently set for the core.
 *
 * @param core The CPU core.
 * @param pFreq The frequency currentlu set
 * @param freq_id The id of the frequency with respect to the order in the freqs internal array
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c core or \c pFreq are NULL.
 *         \retval DVFS_ERROR_INVALID_FREQ_ID the id passed is not valid
 */
int dvfs_core_get_freq(const dvfs_core *core, unsigned int* pFreq, unsigned int freq_id);

/**
 * Gets the number of frequencies available for the core.
 *
 * @param core The CPU core.
 * @param pNbFreq The number of frequencies available for the core
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c core or \c pNbFreq are NULL.
 */
int dvfs_core_get_nb_freqs (const dvfs_core *core, unsigned int* pNbFreq);

/**
 * Gets the ID of the DVFS core
 *
 * @param core The CPU core.
 * @param pID Will be filled with the ID
 *
 * @return \retval DVFS_SUCCESS if everything goes right.
 *         \retval DVFS_ERROR_INVALID_ARG if \c core or \c pNbFreq are NULL.
 */
int dvfs_core_get_id(const dvfs_core* core, unsigned int* pID);
