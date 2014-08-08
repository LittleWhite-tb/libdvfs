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

#include "dvfs_unit.h"

#include <assert.h>
#include <stdlib.h>

#include "dvfs_error.h"

int dvfs_unit_open(dvfs_unit** ppUnit, unsigned int nb_cores, dvfs_core **cores, unsigned int unit_id) {
   assert(ppUnit != NULL);
   assert(cores != NULL);
   if (cores == NULL || ppUnit == NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   *ppUnit = malloc(sizeof(*(*ppUnit)));
   if ( *ppUnit == NULL )
   {
      return DVFS_ERROR_MEM_ALLOC_FAILED;
   }

   (*ppUnit)->nb_cores = nb_cores;
   (*ppUnit)->cores = cores;
   (*ppUnit)->id = unit_id;

   return DVFS_SUCCESS;
}

int dvfs_unit_close(dvfs_unit *unit) {
   unsigned int i;
   int id_result=DVFS_SUCCESS;

   assert(unit != NULL);

   for (i = 0; i < unit->nb_cores; i++) {
      int cresult = dvfs_core_close(unit->cores[i]);
      if ( cresult != DVFS_SUCCESS )
      {
          id_result = cresult;
      }
   }
   free(unit->cores);
   free(unit);

   return id_result;
}

int dvfs_unit_set_gov(const dvfs_unit *unit, const char *gov) {
   unsigned int i;
   int ret = DVFS_SUCCESS;

   assert(unit != NULL);
   assert(gov != NULL);
   if ( unit == NULL || gov == NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   for (i = 0; i < unit->nb_cores; i++) {
      int cret = dvfs_core_set_gov (unit->cores[i], gov);
      if (cret != DVFS_SUCCESS )
      {
         fprintf (stderr, "unitSetGov: error for core #%u\n", i);
         ret=cret; // Report last error to calling function
      }
   }

   return ret;
}

int dvfs_unit_set_freq(const dvfs_unit *unit, unsigned int freq) {
   unsigned int i;
   int ret = DVFS_SUCCESS;

   assert(unit != NULL);
   if ( unit == NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   for (i = 0; i < unit->nb_cores; i++) {
      int cret = dvfs_core_set_freq(unit->cores[i], freq);
      if (cret != DVFS_SUCCESS) {
         fprintf (stderr, "unitSetFreq: error for core #%u\n", i);
         ret=cret;
      }
   }

   return ret;
}

int dvfs_unit_get_core(const dvfs_unit *unit, dvfs_core **ppCore, unsigned int id) {
   unsigned int i;

   assert(unit != NULL);
   assert(ppCore != NULL);
   if ( unit == NULL || ppCore == NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   for (i = 0; i < unit->nb_cores; i++) {
      if (unit->cores[i]->id == id) {
         *ppCore = unit->cores[i];
         return DVFS_SUCCESS;
      }
   }

   return DVFS_ERROR_INVALID_CORE_ID;
}

int dvfs_unit_get_freq(const dvfs_unit *unit, unsigned int* pFreq) {
   unsigned int i=0;

   assert(unit != NULL);
   assert(pFreq != NULL);
   if ( unit == NULL || pFreq == NULL)
   {
       return DVFS_ERROR_INVALID_ARG;
   }

   for (i = 0; i < unit->nb_cores; i++) {
      unsigned int cfreq=0;
      int id_error = dvfs_core_get_current_freq(unit->cores[i],&cfreq);
      if ( id_error != DVFS_SUCCESS )
      {
          return id_error;
      }

      *pFreq = (*pFreq >= cfreq ? *pFreq : cfreq);
   }

   return DVFS_SUCCESS;
}

int dvfs_unit_get_id (const dvfs_unit *unit, unsigned int* pID) {
   assert (unit != NULL);
   if ( unit == NULL )
   {
       return DVFS_ERROR_INVALID_ARG;
   }
   *pID = unit->id;
   return DVFS_SUCCESS;
}
