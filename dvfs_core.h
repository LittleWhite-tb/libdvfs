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
 * @param id The id of the core to control.
 * @param seq True when the frequency transitions must be sequentialized.
 *
 * @return an instanciated Core context for this core. May return NULL in case
 * of error. The error cases are often related to file opening (such as permission
 * denied). In such cases errno is set appropriately.
 *
 * @sa dvfs_core_close()
 */
dvfs_core *dvfs_core_open(unsigned int id, bool seq);

/**
 * Closes properly an opened Core context.
 * Sets back the governor that was in place when opening the context.
 *
 * @param core The core to close.
 */
void dvfs_core_close(dvfs_core *core);

/**
 * Sets the current DVFS governor on the given core to the given buffer.
 *
 * @param core The core on which the governor has to be set.
 * @param buf The pointer to the buffer which will be set to the governor string value
 * @param buf_len The size of the buffer
 *
 * @return Upon successful completion 1 is returned. Otherwise, 0 is returned and errno is set appropriately.
 */
unsigned int dvfs_core_get_gov(const dvfs_core *core, char *buf, size_t buf_len);

/**
 * Changes the governor on the given core.
 *
 * @param core The core on which the governor has to be set.
 * @param gov The governor to set.
 *
 * @return Upon successful completion 1 is returned. Otherwise, 0 is returned and errno is set appropriately.
 */
unsigned int dvfs_core_set_gov(const dvfs_core *core, const char *gov);

/**
 * Sets the frequency for the given core. Assumes that the "userspace" governor
 * has been set, result is unknown otherwise.
 *
 * @param core The related core.
 * @param freq The frequency to set.
 *
 * @return Upon successful completion 1 is returned. Otherwise, 0 is returned and errno is set appropriately.
 */
unsigned int dvfs_core_set_freq(const dvfs_core *core, unsigned int freq);

/**
 * Returns the frequency currently set for the core. Warning, this is not
 * necessarily the frequency currently active for the core as other cores in the
 * same unit may have requested a different frequency. In order to determine the
 * frequency actually set for the core, use instead \p dvfs_unit_get_freq().
 *
 * @param core The CPU core.
 * 
 * @return The frequency selected for this core or 0 in case of error.
 *
 * @sa dvfs_unit_get_freq()
 */
unsigned int dvfs_core_get_current_freq(const dvfs_core *core);

/**
 * Returns the frequency currently set for the core.
 *
 * @param core The CPU core.
 * @param freq_id The id of the frequency with respect to the order in the freqs internal array
 * 
 * @return Upon successful completion the corresponding frequency is return. 0 is returned in case the given freq_id parameter is out of bounds.
 */
unsigned int dvfs_core_get_freq(const dvfs_core *core, unsigned int freq_id);

/**
 * Returns the number of frequencies available for the core.
 *
 * @param core the CPU core.
 *
 * @return The number of frequencies available on this core.
 */
unsigned int dvfs_core_get_nb_freqs (const dvfs_core *core);

