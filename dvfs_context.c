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

#include "dvfs_context.h"
#include "dvfs_error.h"

#include <assert.h>
#include <cpuid.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

static unsigned int get_nb_cores();
static void get_related_cores(unsigned int id, unsigned int **cores, unsigned int *nb_cores);

int dvfs_start(dvfs_ctx** ppCtx, bool seq) {
   unsigned int nb_cores = get_nb_cores();

   if ( ppCtx == NULL )
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   // we can have at most one unit per core
   *ppCtx = malloc(sizeof(*(*ppCtx)));
   if ( *ppCtx == NULL )
   {
       return DVFS_ERROR_MEM_ALLOC_FAILED;
   }

   (*ppCtx)->nb_units = 0;
   (*ppCtx)->units = malloc(nb_cores * sizeof(*(*ppCtx)->units));
   if ( (*ppCtx)->units == NULL )
   {
       dvfs_stop(*ppCtx);
       return DVFS_ERROR_MEM_ALLOC_FAILED;
   }

   unsigned int c;
   for (c = 0; c < nb_cores; c++) {
      unsigned int nb_ucores;
      unsigned int *ucores_ids;
      unsigned int u, uc;
      dvfs_core **ucores;

      // is this core already present in a unit?
      bool known = false;
      for (u = 0; u < (*ppCtx)->nb_units; u++) {
         dvfs_core* pCore = NULL;
         if (dvfs_unit_get_core((*ppCtx)->units[u], &pCore, c) == DVFS_SUCCESS) {
            known = true;
            break;
         }
      }

      if (known) {
         continue;
      }

      // open the related cores ids
      get_related_cores(c, &ucores_ids, &nb_ucores);

      if (ucores_ids == NULL) {
         fprintf(stderr, "Failed to get related cores of core %u\n", c); // TODO
         dvfs_stop(*ppCtx);
         return DVFS_ERROR_RELATED_CORE_UNAVAILABLE;
      }

      // open the cores corresponding to the ids
      ucores = malloc(nb_ucores * sizeof(*ucores));   // freeed on dvfs_unit_close call
      if ( ucores == NULL )
      {
         free(ucores_ids);
         dvfs_stop(*ppCtx);
         return DVFS_ERROR_MEM_ALLOC_FAILED;
      }

      for (uc = 0; uc < nb_ucores; uc++) {
         int result = dvfs_core_open(&ucores[uc],ucores_ids[uc], seq);

         if (result != DVFS_SUCCESS) {
            free (ucores_ids);
            free (ucores);
            dvfs_stop(*ppCtx);
            return result;
         }
      }
      free(ucores_ids);

      // create the unit
      int unit_result = dvfs_unit_open(&(*ppCtx)->units[(*ppCtx)->nb_units], nb_ucores, ucores, (*ppCtx)->nb_units);
      if ( unit_result != DVFS_SUCCESS )
      {
          dvfs_stop(*ppCtx);
          return unit_result;
      }
      (*ppCtx)->nb_units++;
   }

   return DVFS_SUCCESS;
}

int dvfs_stop(dvfs_ctx *ctx) {
   unsigned int i;
   int id_result = DVFS_SUCCESS;

   assert(ctx != NULL);
   if (ctx == NULL )
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   for (i = 0; i < ctx->nb_units; i++)
   {
      if ( ctx->units[i])
      {
          int cres = dvfs_unit_close(ctx->units[i]);
          if ( cres != DVFS_SUCCESS )
          {
              id_result = cres;
          }
      }
   }

   free(ctx->units);
   free(ctx);

   return id_result;
}

int dvfs_has_TB() {
   FILE* pFile = NULL;
   pFile = fopen("/proc/cpuinfo", "r");
   if (pFile == NULL) {
      return DVFS_ERROR_FILE_ERROR;
   }

   int hasTB = DVFS_TB_UNAVAILABLE;
   char buf [2048];
   while (fgets(buf, sizeof(buf), pFile) != NULL) {
      if (!strncmp (buf, "flags", 5)) {
         if (strstr (buf, "ida") != NULL) {
            hasTB = DVFS_TB_AVAILABLE;
            break;
         }
      }
   }

   fclose(pFile);
   return hasTB;
}

int dvfs_set_gov(const dvfs_ctx *ctx, const char *gov) {
   unsigned int i;
   int ret = DVFS_SUCCESS;

   assert(ctx != NULL);
   assert(gov != NULL);
   if ( ctx == NULL || gov == NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   for (i = 0; i < ctx->nb_units; i++) {
      int cret = dvfs_unit_set_gov(ctx->units[i], gov);
      if ( cret != DVFS_SUCCESS )
      {
        ret = cret;
      }
   }

   return ret;
}

int dvfs_set_freq(dvfs_ctx *ctx, unsigned int freq) {
   unsigned int i;
   int ret = DVFS_SUCCESS;

   assert(ctx != NULL);
   if ( ctx == NULL )
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   for (i = 0; i < ctx->nb_units; i++) {
      int cret = dvfs_unit_set_freq(ctx->units[i], freq);
      if ( cret != DVFS_SUCCESS )
      {
          ret = cret;
      }
   }

   return ret;
}

int dvfs_get_core(const dvfs_ctx *ctx, const dvfs_core** ppCore, unsigned int core_id) {
   unsigned int i;

   assert(ctx != NULL);
   assert(ppCore != NULL);
   if ( ctx == NULL || ppCore == NULL )
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   for (i = 0; i < ctx->nb_units; i++) {
      unsigned int j;
      for (j = 0; j < ctx->units[i]->nb_cores; j++) {
         if (ctx->units[i]->cores[j]->id == core_id) {
            *ppCore = ctx->units[i]->cores[j];
             return DVFS_SUCCESS;
         }
      }
   }

   return DVFS_ERROR_INVALID_CORE_ID;
}

int dvfs_get_unit_by_id(const dvfs_ctx* ctx, const dvfs_unit** ppUnit, unsigned int index)
{
    assert(ctx!=NULL);
    assert(ppUnit!=NULL);
    if ( ctx == NULL || ppUnit == NULL)
    {
        return DVFS_ERROR_INVALID_ARG;
    }

    if ( index >= ctx->nb_units )
    {
        return DVFS_ERROR_INVALID_INDEX;
    }

    *ppUnit = ctx->units[index];
    return DVFS_SUCCESS;
}

int dvfs_get_unit_by_core(const dvfs_ctx *ctx, const dvfs_core *core, const dvfs_unit** ppUnit) {
   unsigned int i;

   assert(ctx != NULL);
   assert(core != NULL);
   assert(ppUnit != NULL);
   if ( ctx == NULL || core == NULL || ppUnit == NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   for (i = 0; i < ctx->nb_units; i++) {
      unsigned int j;
      for (j = 0; j < ctx->units[i]->nb_cores; j++) {
         if (ctx->units[i]->cores[j]->id == core->id)
         {
            *ppUnit = ctx->units[i];
            return DVFS_SUCCESS;
         }
      }
   }

   // Sincerely, we should never reach this point
   return DVFS_ERROR_CORE_UNIT_MISMATCH;
}

int dvfs_get_nb_unit(const dvfs_ctx* ctx, unsigned int* pNb)
{
    assert(ctx != NULL);
    assert(pNb != NULL);
    if ( ctx == NULL || pNb == NULL )
    {
        return DVFS_ERROR_INVALID_ARG;
    }

    *pNb = ctx->nb_units;
    return DVFS_SUCCESS;
}

static unsigned int get_nb_cores() {
   unsigned int nb_cores = 0;

   nb_cores = sysconf(_SC_NPROCESSORS_ONLN);
   if (nb_cores < 1) // This sysconf is not always available
   {
      // Second try
      FILE* pFile = NULL;
      pFile = fopen("/proc/cpuinfo", "r");
      if (pFile == NULL) {
         return 0;
      }

      char buf [2048];
      while (fgets(buf, sizeof(buf), pFile) != NULL) {
         if (!strncmp (buf, "processor", 9)) {
            nb_cores++;
         }
      }

      fclose(pFile);
   }

   return nb_cores;
}

static void get_related_cores(unsigned int id, unsigned int **cores, unsigned int *nb_cores) {
   unsigned int val, i;
   char relfile[1024];
   FILE *fd;
   struct stat buf;

   assert(cores != NULL && nb_cores != NULL);

   // all Linux files are broken in some versions... first rely on the manufacturer
   __get_cpuid(0, &i, (unsigned int *) relfile, (unsigned int *) (relfile + 8),
               (unsigned int *) (relfile + 4));
   relfile[12] = '\0';

   // Intel platforms have a single frequency domain
   if (strncmp(relfile, "GenuineIntel", 12) == 0) {
      snprintf(relfile, sizeof(relfile), "/sys/devices/system/cpu/cpu%u/topology/core_siblings_list", id);
   } else {
      // prefer the more recent freq_domain_cpus over related_cpus
      snprintf(relfile, sizeof(relfile), "/sys/devices/system/cpu/cpu%u/cpufreq/freqdomain_cpus", id);
      if (stat(relfile, &buf) < 0) {
         snprintf(relfile, sizeof(relfile), "/sys/devices/system/cpu/cpu%u/cpufreq/related_cpus", id);
      }
   }

   if ((fd = fopen(relfile, "r")) == NULL) {
      *nb_cores = 0;
      *cores = NULL;
      return;
   }

   // parse the file
   // supports space-separated list of values and condensed format
   // (comma-separated list with dash notation for contiguous lists)
   // only counting here
   *nb_cores = 0;
   while (fscanf(fd, "%u", &val) == 1) {
      unsigned int nval;
      char sep;

      int fret = fscanf(fd, "%c", &sep);

      if (fret == EOF || fret == 0 || sep == ' ' || sep == ',' || sep == '\n') {
         (*nb_cores)++;
         continue;
      }

      if (sep != '-') { // Error case ... no format recognized here
         fprintf(stderr, "Illformed topology file: expected '-', read '%c' \n", sep);
         *nb_cores = 0;
         *cores = NULL;
         return;
      }

      // Treat the second file (since first did not match) format (0-N)
      fscanf(fd, "%u", &nval);
      *nb_cores += nval - val + 1;

      fscanf(fd, "%c", &sep);
   }

   *cores = malloc(*nb_cores * sizeof(**cores));
   fseek(fd, 0, SEEK_SET);

   // now fill the array
   i = 0;
   while (fscanf(fd, "%u", &val) == 1 && i < *nb_cores) {
      unsigned int nval;
      char sep;

      fscanf(fd, "%c", &sep);

      if ( sep == '\n' ) // File finished, we guess, we have count everything
      {
         (*cores)[i++] = val;
         break; // Leave
      }
      if (sep == ' ' || sep == ',') {
         (*cores)[i++] = val;
         continue;
      }

      fscanf(fd, "%u", &nval);
      unsigned int j;
      for (j = 0; j < nval - val + 1; j++) {
         (*cores)[i + j] = val + j;
      }
      i += j;

      fscanf(fd, "%c", &sep);
   }

   fclose(fd);
}

