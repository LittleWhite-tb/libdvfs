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

#include "cpu.h"

#include "core.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Opens the CPU context. This open a Core Context for all core available
 *
 * @returns an instanciated CPU context. May return NULL in case
 * of error. The error cases are often related to file opening (like permission denied). An error message will be written
 * on stderr (using fprintf or perror).
 *
 * @see sched_getcpu
 * @sa cpu_closeContext core_openContext
 */
cpu_ctx_t *cpu_openContext()
{
   unsigned int nbCores = 0;
   
   nbCores = sysconf(_SC_NPROCESSORS_ONLN);
   if ( nbCores < 1 ) // This sysconf is not always available
   {
      // Second try
      FILE* pFile = NULL;
      pFile = fopen("/proc/cpuinfo","r");
      if ( pFile == NULL)
      {
         perror("Fail to open /proc/cpuinfo");
         return NULL;
      }
      
      char buf [2048];
      while (fgets(buf,sizeof(buf),pFile) != NULL) {
         if (!strncmp (buf, "processor", 9)) {
            nbCores++;
         }
      }

      fclose(pFile);
   }
   
   cpu_ctx_t *ctx = malloc(sizeof(*ctx));
   ctx->nbCores = nbCores;
   
   // Load core contexts
   {
      ctx->pCoreCtx = malloc(sizeof(*(ctx->pCoreCtx)) * ctx->nbCores);
      
      unsigned int id = 0;
      for ( id = 0 ; id < ctx->nbCores ; id++ )
      {
         ctx->pCoreCtx[id] = core_openContext(id);
         if ( ctx->pCoreCtx[id] == NULL )
         {
            ctx->nbCores = id; // So, we really initialised id cores
            break;
         }
      }
   }
   
   if ( nbCores != ctx->nbCores ) // The initialisation failed
   {
      cpu_closeContext(ctx); // Cleanup
      fprintf(stderr,"Fail to open CPU context\n");
      return NULL;
   }
   
   return ctx;
}

/**
 * Closes properly an opened CPU context.
 * All core contexts for this CPU are closed
 *
 * @param ctx The context to close.
 */
void cpu_closeContext(cpu_ctx_t* ctx)
{
   unsigned int id = 0;
   for ( id = 0 ; id < ctx->nbCores ; id++ )
   {
      core_closeContext(ctx->pCoreCtx[id]);
   }
   
   free(ctx->pCoreCtx);
   free(ctx);
}

/**
 * Changes the governor for the given cpu.
 *
 * @param ctx The context of the CPU on which the governor has to be
 * changed.
 * @param governor The governor to set.
 */
void cpu_setGov(const cpu_ctx_t *ctx, const char* governor)
{
   assert(ctx);
   
   unsigned int id = 0;
   for ( id = 0 ; id < ctx->nbCores ; id++ )
   {
      core_setGov(ctx->pCoreCtx[id],governor);
   }
}

/**
 * Sets the frequency for the given cpu. Assumes that the "userspace" governor
 * has been set.
 *
 * @param ctx The related CPU context.
 * @param freq The frequency to set.
 */
void cpu_setFreq(const cpu_ctx_t *ctx, unsigned int freq)
{
   assert(ctx);
   
   unsigned int id = 0;
   for ( id = 0 ; id < ctx->nbCores ; id++ )
   {
      core_setFreq(ctx->pCoreCtx[id],freq);
   }
}

/**
 * Returns the number of cores for this CPU context.
 *
 * @param ctx The CPU context.
 * @return The number of cores for this CPU context
 */
unsigned int cpu_getNbCores(const cpu_ctx_t* ctx)
{
   assert(ctx);
   
   return ctx->nbCores;
}

/**
 * Returns the core context specified by \a index
 *
 * @param ctx The CPU context.
 * @param index The index of the Core to get.
 * @return A pointeur to the Core context or NULL if the index specified is out of bound
 */
core_ctx_t* cpu_getCore(const cpu_ctx_t* ctx, unsigned int index)
{
   assert(ctx);
   if ( index < ctx->nbCores )
   {
      return ctx->pCoreCtx[index];
   }
   return NULL;
}

/**
 * TurboBoost is implemented as the higher frequency for a CPU. To detect
 * it, we check if /proc/cpuinfo contains the flag "ida" (Intel Dynamic Acceleration)
 * @return 0 if there is no TurboBoost, -1 if an error occured during the check
 */
int hasTurboBoost()
{
   FILE* pFile = NULL;
   pFile = fopen("/proc/cpuinfo","r");
   if ( pFile == NULL)
   {
      perror("Fail to open /proc/cpuinfo");
      return -1;
   }

   int hasTB = 0;
   char buf [2048];
   while (fgets(buf,sizeof(buf),pFile) != NULL) {
      if (!strncmp (buf, "flags", 5)) {
         if (!strstr (buf, "ida")) {
            hasTB = 0;
            break;
         } else {
            hasTB = 1;
            break;
         }
      }
   }

   fclose(pFile);
   return hasTB;
}
