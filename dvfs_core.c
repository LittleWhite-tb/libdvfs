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

#include "dvfs_core.h"


// These patterns should be used in snprintf functions
#define SCALING_GOVERNOR_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_governor"
#define SCALING_CURFREQ_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_cur_freq"
#define SCALING_AVAIL_FREQ_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_available_frequencies"
#define SCALING_SETSPEED_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_setspeed"

dvfs_core *dvfs_core_open(unsigned int id) {
   char fname [256];
   char freqs [1024];
   char *strtokctx, *tmpstr;
   FILE *fd;
   unsigned int i;

   dvfs_core *core = malloc(sizeof(*core));
   
   // A well initialized struct avoids tons of errors, trust me
   core->id = id;
   core->nb_freqs = 0;
   core->freqs = NULL;
   core->fd_getf = NULL;
   core->fd_setf = NULL;
   memset (core->init_gov, 0, sizeof (core->init_gov));
   core->init_freq = 0;

   // Paranoid: Make sure the fname buffer is long enough
   assert (sizeof (SCALING_GOVERNOR_FILE_PATTERN) <= sizeof (fname));
   
   /* fetch  the initial governor and frequency */
   snprintf (fname, sizeof (fname), SCALING_GOVERNOR_FILE_PATTERN, id);

   fd = fopen(fname, "r");
   if (fd == NULL) {
      free(core);
      return NULL;
   }
   fscanf(fd, "%127s", core->init_gov);
   fclose(fd);

   if (!strcmp(core->init_gov, "userspace")) {
      snprintf (fname, sizeof (fname), SCALING_CURFREQ_FILE_PATTERN, id);
      fd = fopen(fname, "r");
      if (fd == NULL) {
         free(core);
         return NULL;
      }
      fscanf(fd, "%u", &core->init_freq);
      fclose(fd);
   }

   // Paranoid: Make sure the fname buffer is long enough
   assert (sizeof (SCALING_AVAIL_FREQ_FILE_PATTERN) <= sizeof (fname));

   /* parse all the frequencies */
   snprintf (fname, sizeof (fname), SCALING_AVAIL_FREQ_FILE_PATTERN, id);
   fd = fopen(fname, "r");
   if (fd == NULL) {
      free(core);
      return NULL;
   }

   // Set freqs memory to zero 
   if (fgets(freqs, sizeof(freqs), fd) == NULL) {
      fclose(fd);
      free(core);
      return NULL;
   }
   fclose(fd); 

   bool inFreq = false;
   // Count freqs number
   for (tmpstr = freqs; *tmpstr; tmpstr++) {
      if (*tmpstr >= '0' && *tmpstr <= '9') {
         if (inFreq) {
            continue;
         }
         core->nb_freqs++;
         inFreq = true;
      } else {
         inFreq = false;
      }
   }
   
   // Paranoid: No need to syscall malloc if no freqs are available
   assert (core->nb_freqs > 0);

   core->freqs = malloc(core->nb_freqs * sizeof(*core->freqs));

   for (i = 0, tmpstr = strtok_r(freqs, " ", &strtokctx);
        i < core->nb_freqs && tmpstr != NULL;
        i++, tmpstr = strtok_r(NULL, " ", &strtokctx))
   {
      char *end;
      core->freqs[core->nb_freqs - i - 1] = strtol (tmpstr, &end, 10);
      
      // Paranoid: Check that what we have read in the file is valid
      assert (end != tmpstr);
   }
   assert (i == core->nb_freqs);

   // Paranoid: Make sure the fname buffer is long enough
   assert (sizeof (SCALING_SETSPEED_FILE_PATTERN) <= sizeof (fname));
  
   // open the frequency setter file
   snprintf (fname, sizeof (fname), SCALING_SETSPEED_FILE_PATTERN, id);
   core->fd_setf = fopen(fname, "w");
   if (core->fd_setf == NULL) {
      dvfs_core_close(core);
      return NULL;
   }

   // Paranoid: Make sure the fname buffer is long enough
   assert (sizeof (SCALING_CURFREQ_FILE_PATTERN) <= sizeof (fname));

   // same for the frequency getter file
   snprintf(fname, sizeof(fname), SCALING_CURFREQ_FILE_PATTERN, id);
   core->fd_getf = fopen(fname, "r");
   if (core->fd_getf == NULL) {
      dvfs_core_close(core);
      return NULL;
   }

   return core;
}

void dvfs_core_close(dvfs_core *core) {
   assert (core != NULL);
   // restore the previous state
   dvfs_core_set_gov(core, core->init_gov);

   if (strcmp(core->init_gov, "userspace") == 0) {
      dvfs_core_set_freq(core, core->init_freq);
   }

   free(core->freqs), core->freqs = NULL;
   if (core->fd_setf != NULL) {
      fclose(core->fd_setf), core->fd_setf = NULL;
   }

   if (core->fd_getf != NULL) {
      fclose(core->fd_getf), core->fd_getf = NULL;
   }

   free(core);
}

unsigned int dvfs_core_set_gov(const dvfs_core *core, const char *gov) {
   char fname [256];
   FILE *fd;

   assert (core != NULL); 

   // Paranoid: Make sure the fname buffer is long enough
   assert (sizeof (SCALING_GOVERNOR_FILE_PATTERN) <= sizeof (fname));

   snprintf (fname, sizeof (fname), SCALING_GOVERNOR_FILE_PATTERN, core->id);
   fd = fopen (fname, "w");
   if (fd == NULL) {
      return 0;
   }

   if (fwrite (gov, sizeof (*gov), strlen (gov) + 1, fd) < strlen (gov) + 1) {
      fclose (fd);
      return 0;
   }

   if (fflush (fd) != 0) {
      return 0;
   }

   fclose (fd);
   return 1;
}

unsigned int dvfs_core_set_freq(const dvfs_core *core, unsigned int freq) {
   assert (core != NULL);
   
   // If fd_freq has not been opened yet
   if (core->fd_setf == NULL) {
      return 0;
   }
   // check that the frequency asked is available
#ifndef NDEBUG
   unsigned int i;
   short freqIsValid = 0;
   for (i = 0; i < core->nb_freqs; i++) {
      if (core->freqs [i] == freq) {
         freqIsValid = 1;
         break;
      }
   }

   if (!freqIsValid) {
      fprintf (stderr, "Freq %u is invalid\n", freq);
   }
   assert (freqIsValid);
#endif

   if (fprintf(core->fd_setf, "%u", freq) < 0) {
      return 0;
   }

   if (fflush (core->fd_setf) != 0) {
      return 0;
   }

   return 1;
}

unsigned int dvfs_core_get_current_freq(const dvfs_core *core) {
   unsigned int res;

   assert (core != NULL);

   if (fscanf(core->fd_getf, "%u", &res) < 0) {
      return 0;
   }

   return res;
}

unsigned int dvfs_core_get_freq (const dvfs_core *core, unsigned int freq_id) {
   assert (core != NULL);
   
   if (freq_id >= core->nb_freqs) {
      return 0;
   }

   return core->freqs [freq_id];
}

unsigned int dvfs_core_get_nb_freqs (const dvfs_core *core) {
   assert (core != NULL);
   
   return core->nb_freqs;
}
