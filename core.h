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

#ifndef CORE_H
#define CORE_H

#include <stdio.h>

/**
 * Core Context
 * A core context allows to control CPU Core governor and frequency.
 * Morever, you can get the list of frequencies available for this core with
 * \a core_getNbFreqs and \a core_getFreq.
 */
typedef struct {
   unsigned int cpuId; //!< core id
   unsigned int nbFreqs; //!< Number of frequency available for this core
   unsigned int *freqs; //!< List of frequency available for this core

   FILE *freqFd; //!< File descriptor on the set_speed file 
   unsigned int curFreq; //!< Actual core freqency

   char initGov[128]; //!< governor used when core get initialised
   unsigned int initFreq; //!< freqency used when core get initialised
} core_ctx_t;

core_ctx_t *core_openContext(unsigned int cpuId);
void core_closeContext(core_ctx_t *ctx);

void core_setGov(const core_ctx_t *ctx, const char *governor);

void core_setFreq(const core_ctx_t *ctx, unsigned int freq);
unsigned int core_getNbFreqs(const core_ctx_t *ctx);
unsigned int core_getFreq(const core_ctx_t *ctx, unsigned int freqId);

#endif
