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

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core.h"


// These patterns should be used in snprintf functions
#define SCALING_GOVERNOR_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_governor"
#define SCALING_CURFREQ_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_cur_freq"
#define SCALING_AVAIL_FREQ_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_available_frequencies"
#define SCALING_SETSPEED_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_setspeed"

/**
 * Opens the Core context for the given core ID.
 *
 * @param cpuId The id of the desired core.
 * @returns an instanciated Core context for this core. May return NULL in case
 * of error.
 *
 * @see sched_getcpu
 * @sa dvfs_closeContext
 */
core_ctx_t *core_openContext(unsigned int cpuId) {
   char fname [256];
   char freqs[1024];
   char *strtokctx, *tmpstr;
   FILE *fd;
   unsigned int i;

   core_ctx_t *ctx = malloc(sizeof(*ctx));

   /* fetch  the initial governor and frequency */
   snprintf (fname, sizeof (fname),SCALING_GOVERNOR_FILE_PATTERN, cpuId);

   fd = fopen(fname, "r");
   if (fd == NULL) {
      perror("Failed to open governor file");
      free(ctx);
      return NULL;
   }
   fscanf(fd, "%127s", ctx->initGov);
   fclose(fd);

   if (!strcmp(ctx->initGov, "userspace")) {
      snprintf (fname, sizeof (fname),SCALING_CURFREQ_FILE_PATTERN, cpuId);
      fd = fopen(fname, "r");
      if (fd == NULL) {
         perror("Failed to open frequency file");
         free(ctx);
         return NULL;
      }
      fscanf(fd, "%u", &ctx->initFreq);
      fclose(fd);
   }

   /* parse all the frequencies */
   snprintf (fname, sizeof (fname),SCALING_AVAIL_FREQ_FILE_PATTERN, cpuId);
   fd = fopen(fname, "r");
   if (fd == NULL) {
      perror("Failed to open available frequencies");
      free(ctx);
      return NULL;
   }
   if (fgets(freqs, sizeof(freqs), fd) == NULL) {
      perror("Failed to read available frequencies");
      fclose(fd);
      free(ctx);
      return NULL;
   }
   fclose(fd);

   ctx->nbFreqs = 0;
   bool inFreq = false;
   // Count freqs number
   for (tmpstr = freqs; *tmpstr; tmpstr++) {
      if (*tmpstr >= '0' && *tmpstr <= '9') {
         if (inFreq) {
            continue;
         }
         ctx->nbFreqs++;
         inFreq = true;
      } else {
         inFreq = false;
      }
   }
   ctx->freqs = malloc(ctx->nbFreqs * sizeof(*ctx->freqs));

   for (i = 0, tmpstr = strtok_r(freqs, " ", &strtokctx);
        i < ctx->nbFreqs && tmpstr != NULL;
        i++, tmpstr = strtok_r(NULL, " ", &strtokctx))
   {
      ctx->freqs[ctx->nbFreqs - i - 1] = atol(tmpstr);
   }
   assert(i == ctx->nbFreqs);
  
   // open the frequency setter file
   snprintf (fname, sizeof (fname),SCALING_SETSPEED_FILE_PATTERN, cpuId);
   ctx->freqFd = fopen(fname, "w");
   if (ctx->freqFd == NULL) {
      perror("Failed to open frequency setting file");
      core_closeContext(ctx);
      return NULL;
   }

   ctx->cpuId = cpuId;
   ctx->curFreq = 0;

   return ctx;
}

/**
 * Closes properly an opened Core context.
 * Sets back the governor that was in place when opening the context
 *
 * @param ctx The context to close.
 */
void core_closeContext(core_ctx_t *ctx) {
   assert(ctx != NULL);

   // restore the previous state
   core_setGov(ctx, ctx->initGov);
   if (strcmp(ctx->initGov, "userspace") == 0) {
      core_setFreq(ctx, ctx->initFreq);
   }

   free(ctx->freqs);
   if ( ctx->freqFd != NULL )
   {
      fclose(ctx->freqFd);
   }
   free(ctx);
}


/**
 * Changes the governor for the given core.
 *
 * @param ctx The context of the Core context on which the governor has to be
 * changed.
 * @param governor The governor to set.
 */
void core_setGov(const core_ctx_t *ctx, const char *governor) {
   char fname [256];
   FILE *fd;

   assert(ctx != NULL);

   snprintf (fname, sizeof (fname),SCALING_GOVERNOR_FILE_PATTERN, ctx->cpuId);
   fd = fopen(fname, "w");
   if (fd == NULL) {
      perror("Failed to open governor setter file");
      return;
   }
   if (fwrite(governor, sizeof(*governor), strlen(governor) + 1, fd) < strlen(governor) + 1) {
      perror("Failed to set the governor");
      return;
   }
   fflush(fd);
   fclose(fd);
}

/**
 * Sets the frequency for the given core. Assumes that the "userspace" governor
 * has been set.
 *
 * @param ctx The related Core context.
 * @param freq The frequency to set.
 */
void core_setFreq(const core_ctx_t *ctx, unsigned int freq) {
   assert(ctx != NULL);

   if (fprintf(ctx->freqFd, "%u", freq) < 0) {
      perror("Failed to set frequency");
      return;
   }
   fflush(ctx->freqFd);
}

/**
 * Returns the number of available frequencies in the given Core context.
 *
 * @param ctx The Core context.
 * @return The number of different frequencies in the context.
 */
unsigned int core_getNbFreqs(const core_ctx_t *ctx) {
   assert(ctx != NULL);
   
   return ctx->nbFreqs;
}

/**
 * Returns the frequencies available for the given context. Frequencies are
 * sorted: the minimal frequency is always at position 0.
 *
 * @param ctx The Core context.
 * @param freqId The position of the frequency in the context (< nbFreqs).
 *
 * @return The frequency at the given position in the context.
 */
unsigned int core_getFreq(const core_ctx_t *ctx, unsigned int freqId) {
   assert(ctx != NULL);
   assert(freqId < ctx->nbFreqs);

   return ctx->freqs[freqId];
}
