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

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dvfs_core.h"
#include "dvfs_error.h"

// Semaphore name
#define SEM_NAME "/libdvfsSeqSem"

// These patterns should be used in snprintf functions
#define SCALING_GOVERNOR_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_governor"
#define SCALING_CURFREQ_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_cur_freq"
#define SCALING_AVAIL_FREQ_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_available_frequencies"
#define SCALING_SETSPEED_FILE_PATTERN "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_setspeed"

// Macros for wait and post on semaphore
#define SAFE_SEM_POST(semaphore) { if (semaphore != NULL) sem_post(semaphore); }
#define SAFE_SEM_WAIT(semaphore) { if (semaphore != NULL) sem_wait(semaphore); }

static void init_dvfs_core(dvfs_core* pCore, unsigned int id, bool seq)
{
    assert(pCore);

    // A well initialized struct avoids tons of errors, trust me
    pCore->id = id;
    pCore->nb_freqs = 0;
    pCore->freqs = NULL;
    pCore->fd_getf = NULL;
    pCore->fd_setf = NULL;
    memset (pCore->init_gov, 0, sizeof (pCore->init_gov));
    pCore->init_freq = 0;
    pCore->sem = NULL;

    // open / create the semaphore
    if (seq) {
       sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0600, 1);
       if (sem == SEM_FAILED)
       {
          perror("[LIBDVFS][WARNING] Failed  to open libdvfs semaphore (frequency transitions will not be sequentialized)");
          // Even if the semaphore failed, we will continue.
       }
       else
       {
          pCore->sem = sem;
       }
    }
}

static int read_governor(dvfs_core* pCore)
{
    char fname [256] = {0};

    assert(pCore);

    // Paranoid: Make sure the fname buffer is long enough
    assert (sizeof (SCALING_GOVERNOR_FILE_PATTERN) <= sizeof (fname));

    /* fetch  the initial governor and frequency */
    if ( snprintf (fname, sizeof (fname), SCALING_GOVERNOR_FILE_PATTERN, pCore->id) >= (int)sizeof(fname) )
    {
        return DVFS_ERROR_BUFFER_TOO_SHORT;
        // No cleanup to do
    }

    SAFE_SEM_WAIT(pCore->sem);
    FILE* fd = fopen(fname, "r");
    if (fd == NULL)
    {
       SAFE_SEM_POST(pCore->sem);
       return DVFS_ERROR_FILE_ERROR;
    }
    fscanf(fd, "%127s", pCore->init_gov);
    fclose(fd);

    SAFE_SEM_POST(pCore->sem);

    return DVFS_SUCCESS;
}

static int read_cur_freq(dvfs_core* pCore)
{
    char fname [256] = {0};

    assert(pCore);

    assert (sizeof (SCALING_CURFREQ_FILE_PATTERN) <= sizeof (fname));
    if ( snprintf (fname, sizeof (fname), SCALING_CURFREQ_FILE_PATTERN, pCore->id) >= (int)sizeof(fname) )
    {
        return DVFS_ERROR_BUFFER_TOO_SHORT;
        // No cleanup to do here
    }

    SAFE_SEM_WAIT(pCore->sem);
    FILE* fd = fopen(fname, "r");
    if (fd == NULL)
    {
        SAFE_SEM_POST(pCore->sem);
        return DVFS_ERROR_FILE_ERROR;
    }
    fscanf(fd, "%u", &pCore->init_freq);
    fclose(fd);

    SAFE_SEM_POST(pCore->sem);

    return DVFS_SUCCESS;
}

int read_available_freq(dvfs_core* pCore)
{
    char fname [256] = {0};

    assert(pCore);
    // Paranoid: Make sure the fname buffer is long enough
    assert (sizeof (SCALING_AVAIL_FREQ_FILE_PATTERN) <= sizeof (fname));

    /* parse all the frequencies */
    if ( snprintf (fname, sizeof (fname), SCALING_AVAIL_FREQ_FILE_PATTERN, pCore->id) >= (int)sizeof(fname))
    {
        return DVFS_ERROR_BUFFER_TOO_SHORT;
    }

    FILE* fd = fopen(fname, "r");
    if (fd == NULL)
    {
        return DVFS_ERROR_FILE_ERROR;
    }

    // Set freqs memory to zero
    char freqs [1024] = {0};
    char* fgets_error = fgets(freqs, sizeof(freqs), fd);
    fclose(fd);

    if (fgets_error == NULL)
    {
       return DVFS_ERROR_FILE_ERROR;
    }

    char *tmpstr=NULL;
    bool inFreq = false;
    // Count freqs number
    for (tmpstr = freqs; *tmpstr; tmpstr++) {
       if (*tmpstr >= '0' && *tmpstr <= '9') {
          if (inFreq) {
             continue;
          }
          pCore->nb_freqs++;
          inFreq = true;
       } else {
          inFreq = false;
       }
    }

    // Paranoid: No need to syscall malloc if no freqs are available
    assert (pCore->nb_freqs > 0);

    pCore->freqs = malloc(pCore->nb_freqs * sizeof(*pCore->freqs));
    if ( pCore->freqs == NULL )
    {
        return DVFS_ERROR_MEM_ALLOC_FAILED;
    }

    unsigned int i=0;
    char *strtokctx=NULL;
    for (i = 0, tmpstr = strtok_r(freqs, " ", &strtokctx);
         i < pCore->nb_freqs && tmpstr != NULL;
         i++, tmpstr = strtok_r(NULL, " ", &strtokctx))
    {
       char *end;
       pCore->freqs[pCore->nb_freqs - i - 1] = strtol (tmpstr, &end, 10);

       // Paranoid: Check that what we have read in the file is valid
       assert (end != tmpstr);
    }
    assert (i == pCore->nb_freqs);

    return DVFS_SUCCESS;
}

int dvfs_core_open(dvfs_core** pCore, unsigned int id, bool seq) {
   char fname [256] = {0};

   int id_error=DVFS_SUCCESS;

   if ( pCore == NULL )
   {
        return DVFS_ERROR_INVALID_ARG;
   }

   *pCore = malloc(sizeof(dvfs_core));
   if ( pCore == NULL )
   {
       return DVFS_ERROR_MEM_ALLOC_FAILED;
   }

   init_dvfs_core(*pCore,id,seq);

   // Gets initial governor (to put it back later)
   id_error = read_governor(*pCore);
   if ( id_error != DVFS_SUCCESS )
   {
       dvfs_core_close(*pCore);
       return id_error;
   }

   if (!strcmp((*pCore)->init_gov, "userspace")) // If it was userspace, we have
                                             // to gets initial frequency
                                             // to put it back later
   {
      id_error = read_cur_freq(*pCore);
      if ( id_error != DVFS_SUCCESS )
      {
          dvfs_core_close(*pCore);
          return id_error;
      }
   }

   id_error = read_available_freq(*pCore);
   if ( id_error != DVFS_SUCCESS)
   {
       dvfs_core_close(*pCore);
       return id_error;
   }

   // Paranoid: Make sure the fname buffer is long enough
   assert (sizeof (SCALING_SETSPEED_FILE_PATTERN) <= sizeof (fname));

   // open the frequency setter file
   if ( snprintf (fname, sizeof (fname), SCALING_SETSPEED_FILE_PATTERN, id) >= (int)sizeof(fname))
   {
      dvfs_core_close(*pCore);
      return DVFS_ERROR_BUFFER_TOO_SHORT;
   }

   (*pCore)->fd_setf = fopen(fname, "w");
   // don't check the result here to allow instantiating the library without any
   // write access. Only set freq will fail (with no trouble).

   // Paranoid: Make sure the fname buffer is long enough
   assert (sizeof (SCALING_CURFREQ_FILE_PATTERN) <= sizeof (fname));

   // same for the frequency getter file
   if ( snprintf(fname, sizeof(fname), SCALING_CURFREQ_FILE_PATTERN, id) >= (int)sizeof(fname) )
   {
      dvfs_core_close(*pCore);
      return DVFS_ERROR_BUFFER_TOO_SHORT;
   }

   (*pCore)->fd_getf = fopen(fname, "r");
   if ((*pCore)->fd_getf == NULL) {
      dvfs_core_close(*pCore);
      return DVFS_ERROR_FILE_ERROR;
   }

   return DVFS_SUCCESS;
}

int dvfs_core_close(dvfs_core *core) {
   assert (core != NULL);
   if (core==NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }


   // restore the previous state
   if (core->init_gov)
   {
       dvfs_core_set_gov(core, core->init_gov);

       if (strcmp(core->init_gov, "userspace") == 0) {
          dvfs_core_set_freq(core, core->init_freq);
       }
   }

   free(core->freqs), core->freqs = NULL;
   if (core->fd_setf != NULL) {
      fclose(core->fd_setf), core->fd_setf = NULL;
   }

   if (core->fd_getf != NULL) {
      fclose(core->fd_getf), core->fd_getf = NULL;
   }

   // close the semaphore
   if (core->sem != NULL) {
      sem_close(core->sem);
      sem_unlink(SEM_NAME);
   }

   free(core);

   return DVFS_SUCCESS;
}

int dvfs_core_get_gov (const dvfs_core *core, char *buf, size_t buf_len) {
   char fname [256]={0};
   FILE *fd=NULL;
   assert (core != NULL);
   if (core==NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   assert (sizeof (SCALING_GOVERNOR_FILE_PATTERN) <= sizeof (fname));
   if ( snprintf (fname, sizeof (fname), SCALING_GOVERNOR_FILE_PATTERN, core->id) >= (int)sizeof(fname))
   {
       return DVFS_ERROR_BUFFER_TOO_SHORT;
   }

   SAFE_SEM_WAIT(core->sem);
   fd = fopen (fname, "r");
   if (fd == NULL)
   {
      SAFE_SEM_POST(core->sem);
      return DVFS_ERROR_FILE_ERROR;
   }

   char* fgets_result = fgets (buf, buf_len, fd);
   fclose (fd);
   SAFE_SEM_POST(core->sem);

   if (fgets_result == NULL)
   {
      return DVFS_ERROR_FILE_ERROR;
   }

   return DVFS_SUCCESS;
}

int dvfs_core_set_gov(const dvfs_core *core, const char *gov) {
   char fname [256]={0};
   FILE *fd=NULL;

   assert (core != NULL);
   if (core==NULL || gov == NULL)
   {
      return DVFS_ERROR_INVALID_ARG;
   }

   // Paranoid: Make sure the fname buffer is long enough
   assert (sizeof (SCALING_GOVERNOR_FILE_PATTERN) <= sizeof (fname));
   if ( snprintf (fname, sizeof (fname), SCALING_GOVERNOR_FILE_PATTERN, core->id) >= (int)sizeof(fname))
   {
       return DVFS_ERROR_BUFFER_TOO_SHORT;
   }

   SAFE_SEM_WAIT(core->sem);

   fd = fopen (fname, "w");
   if (fd == NULL)
   {
      SAFE_SEM_POST(core->sem);
      return DVFS_ERROR_FILE_ERROR;
   }

   if (fwrite (gov, sizeof (*gov), strlen (gov) + 1, fd) < strlen (gov) + 1)
   {
      fclose (fd);
      SAFE_SEM_POST(core->sem);
      return DVFS_ERROR_FILE_ERROR;
   }

   int fflush_error = fflush (fd);
   fclose(fd);
   SAFE_SEM_POST(core->sem);
   if (fflush_error != 0) {
      return DVFS_ERROR_FILE_ERROR;
   }

   return DVFS_SUCCESS;
}

int dvfs_core_set_freq(const dvfs_core *core, unsigned int freq) {
   assert (core != NULL);
   if (core==NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   // If fd_freq has not been opened yet
   if (core->fd_setf == NULL)
   {
      return DVFS_ERROR_SET_FREQ_FILE;
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

   if (!freqIsValid)
   {
      return DVFS_ERROR_INVALID_FREQ;
   }
   assert (freqIsValid);
#endif

   SAFE_SEM_WAIT(core->sem);
   if (fprintf(core->fd_setf, "%u", freq) < 0) {
      SAFE_SEM_POST(core->sem);
      return DVFS_ERROR_FILE_ERROR;
   }

   int fflush_result = fflush (core->fd_setf);
   SAFE_SEM_POST(core->sem);
   if (fflush_result != 0)
   {
      return DVFS_ERROR_FILE_ERROR;
   }

   return DVFS_SUCCESS;
}

int dvfs_core_get_current_freq(const dvfs_core *core, unsigned int* pFreq) {
   assert (core != NULL);
   assert (pFreq != NULL);
   if (core == NULL || pFreq == NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   SAFE_SEM_WAIT(core->sem);
   if (fscanf(core->fd_getf, "%u", pFreq) < 0) {
      SAFE_SEM_POST(core->sem);
      return DVFS_ERROR_FILE_ERROR;
   }
   SAFE_SEM_POST(core->sem);

   return DVFS_SUCCESS;
}

int dvfs_core_get_freq (const dvfs_core *core, unsigned int* pFreq, unsigned int freq_id) {
   assert (core != NULL);
   assert (pFreq != NULL);
   if (core == NULL || pFreq == NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   if (freq_id >= core->nb_freqs)
   {
      return DVFS_ERROR_INVALID_FREQ_ID;
   }

   *pFreq = core->freqs [freq_id];
   return DVFS_SUCCESS;
}

int dvfs_core_get_nb_freqs (const dvfs_core *core, unsigned int *pNbFreq) {
   assert (core != NULL);
   assert(pNbFreq != NULL);
   if ( core == NULL || pNbFreq==NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   *pNbFreq=core->nb_freqs;
   return DVFS_SUCCESS;
}

int dvfs_core_get_id(const dvfs_core* core, unsigned int* pID)
{
    assert(core != NULL);
    assert(pID != NULL);
    if ( core == NULL || pID == NULL )
    {
        return DVFS_ERROR_INVALID_ARG;
    }

    *pID = core->id;
    return DVFS_SUCCESS;
}
