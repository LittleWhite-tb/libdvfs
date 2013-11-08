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

#include <stdio.h>

/**
 * @file core.h
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

   FILE *fd;               //!< File descriptor toward the \c set_speed file 
   unsigned int cur_freq;  //!< Last requested core freqency

   char init_gov[128];     //!< Governor used when core get initialised
   unsigned int init_freq; //!< Freqency used when core get initialised
} dvfs_core;

/**
 * Opens the Core context for the given core ID.
 *
 * @param id The id of the core to control.
 *
 * @return an instanciated Core context for this core. May return NULL in case
 * of error. The error cases are often related to file opening (like permission
 * denied). An error message will be written on stderr (using fprintf or
 * perror).
 *
 * @sa dvfs_core_close()
 */
dvfs_core *dvfs_core_open(unsigned int id);

/**
 * Closes properly an opened Core context.
 * Sets back the governor that was in place when opening the context.
 *
 * @param core The core to close.
 */
void dvfs_core_close(dvfs_core *core);

/**
 * Changes the governor on the given core.
 *
 * @param core The core on which the governor has to be set.
 * @param gov The governor to set.
 */
void dvfs_core_set_gov(const dvfs_core *core, const char *gov);

/**
 * Sets the frequency for the given core. Assumes that the "userspace" governor
 * has been set, result is unknown otherwise.
 *
 * @param core The related core.
 * @param freq The frequency to set.
 */
void dvfs_core_set_freq(dvfs_core *core, unsigned int freq);

