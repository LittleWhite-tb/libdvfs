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

#include "dvfs_context.h"

#include <assert.h>
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

dvfs_ctx *dvfs_start() {
   unsigned int nb_cores = get_nb_cores();

   // we can have at most one unit per core
   dvfs_ctx *ctx = malloc(sizeof(*ctx));
   ctx->nb_units = 0;
   ctx->units = malloc(nb_cores * sizeof(*ctx->units));

   unsigned int c;
   for (c = 0; c < nb_cores; c++) {
      unsigned int nb_ucores;
      unsigned int *ucores_ids;
      unsigned int u, uc;
      dvfs_core **ucores;

      // is this core already present in a unit?
      bool known = false;
      for (u = 0; u < ctx->nb_units; u++) {
         if (dvfs_unit_contains(ctx->units[u], c)) {
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
         fprintf(stderr, "Failed to get related cores of core %u\n", c);
         free(ctx->units);
         return NULL;
      }

      // open the cores corresponding to the ids
      ucores = malloc(nb_ucores * sizeof(*ucores));   // freeed on dvfs_unit_close call
      for (uc = 0; uc < nb_ucores; uc++) {
         ucores[uc] = dvfs_core_open(ucores_ids[uc]);

         if (ucores[uc] == NULL) {
            nb_ucores = uc;
            break;
         }
      }
      free(ucores_ids);

      // create the unit
      ctx->units[ctx->nb_units++] = dvfs_unit_open(nb_ucores, ucores);
   }
      
   return ctx;
}

void dvfs_stop(dvfs_ctx *ctx) {
   unsigned int i;

   assert(ctx != NULL);

   for (i = 0; i < ctx->nb_units; i++) {
      dvfs_unit_close(ctx->units[i]);
   }
   free(ctx->units);
   free(ctx);
}

bool dvfs_has_TB() {
   FILE* pFile = NULL;
   pFile = fopen("/proc/cpuinfo", "r");
   if (pFile == NULL) {
      perror("Fail to open /proc/cpuinfo");
      return false;
   }

   bool hasTB = false;
   char buf [2048];
   while (fgets(buf, sizeof(buf), pFile) != NULL) {
      if (!strncmp (buf, "flags", 5)) {
         if (strstr (buf, "ida") != NULL) {
            hasTB = true;
            break;
         }
      }
   }

   fclose(pFile);
   return hasTB;
}

/**
 * Sets the provided governor on all the DVFS units.
 *
 * @param dvfs The DVFS context as provided by dvfs_start
 * @param gov The new governor to set
 */
void dvfs_set_gov(const dvfs_ctx *ctx, const char *gov) {
   unsigned int i;

   assert(ctx != NULL);

   for (i = 0; i < ctx->nb_units; i++) {
      dvfs_unit_set_gov(ctx->units[i], gov);
   }
}

/**
 * Sets the given frequency on all the DVFS units. The effects are unknown if
 * the current governor is not "userspace".
 *
 * @param dvfs The DVFS context as provided by dvfs_start
 * @param freq The new frequency to set.
 */
void dvfs_set_freq(dvfs_ctx *ctx, unsigned int freq) {
   unsigned int i;

   assert(ctx != NULL);

   for (i = 0; i < ctx->nb_units; i++) {
      dvfs_unit_set_freq(ctx->units[i], freq);
   }
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
         perror("Fail to open /proc/cpuinfo");
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
   struct utsname un;
   FILE *fd;

   assert(cores != NULL && nb_cores != NULL);

   uname(&un);

   // related_cpus disappears from 3.9 to 3.10 kernel versions...
   // assume shared freq domain at the CPU level there
   if (!strncmp(un.release, "3.9", 3) || !strncmp(un.release, "3.10", 4)) {
      snprintf(relfile, sizeof(relfile), "/sys/devices/system/cpu/cpu%u/topology/core_siblings_list", id);
   } else {
      struct stat buf;
      snprintf(relfile, sizeof(relfile), "/sys/devices/system/cpu/cpu%u/cpufreq/freqdomain_cpus", id);

      // old filename before 3.9 kernels
      if (stat(relfile, &buf) < 0) {
         snprintf(relfile, sizeof(relfile), "/sys/devices/system/cpu/cpu%u/cpufreq/related_cpus", id);
      }
   }
   
   if ((fd = fopen(relfile, "r")) == NULL) {
      perror("Failed to read related core file");
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

      fscanf(fd, "%c", &sep);
      
      // Try to treat the first file format (0 1 2 ... N)
      if ( sep == '\n' ) // File finished, we guess, we have count everything
      {
         (*nb_cores)++; // Last core to count
         break; // Leave
      }
      if (sep == ' ' || sep == ',') {
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

