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

#ifndef CPU_H
#define CPU_H

#include "core.h"

/**
 * CPU Context
 * A CPU context is composed of several Core context
 * 
 * \sa core_ctx_t
 */
typedef struct {
   unsigned int nbCores; //!< Number of cores for this CPU
   core_ctx_t** pCoreCtx; //!< List of Core contexte for this CPU
} cpu_ctx_t;

cpu_ctx_t *cpu_openContext();
void cpu_closeContext(cpu_ctx_t* ctx);

void cpu_setGov(const cpu_ctx_t *ctx, const char* governor);
void cpu_setFreq(const cpu_ctx_t *ctx, unsigned int freq);

unsigned int cpu_getNbCores(const cpu_ctx_t* ctx);
core_ctx_t* cpu_getCore(const cpu_ctx_t* ctx, unsigned int index);

int hasTurboBoost();

#endif
