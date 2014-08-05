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

dvfs_unit *dvfs_unit_open(unsigned int nb_cores, dvfs_core **cores, unsigned int unit_id) {
   assert(cores != NULL);

   dvfs_unit *res = malloc(sizeof(*res));
   res->nb_cores = nb_cores;
   res->cores = cores;
   res->id = unit_id;
   return res;
}

void dvfs_unit_close(dvfs_unit *unit) {
   unsigned int i;

   assert(unit != NULL);

   for (i = 0; i < unit->nb_cores; i++) {
      dvfs_core_close(unit->cores[i]);
   }
   free(unit->cores);
   free(unit);
}

unsigned int dvfs_unit_set_gov(const dvfs_unit *unit, const char *gov) {
   unsigned int i;
   short ret = 1;

   assert(unit != NULL);

   for (i = 0; i < unit->nb_cores; i++) {
      ret &= dvfs_core_set_gov (unit->cores[i], gov);
      if (!ret) {
         fprintf (stderr, "unitSetGov: error for core #%u\n", i);
      }
   }

   return ret;
}

unsigned int dvfs_unit_set_freq(const dvfs_unit *unit, unsigned int freq) {
   unsigned int i;
   short ret = 1;

   assert(unit != NULL);

   for (i = 0; i < unit->nb_cores; i++) {
      ret &= dvfs_core_set_freq(unit->cores[i], freq);
      if (!ret) {
         fprintf (stderr, "unitSetFreq: error for core #%u\n", i);
      }
   }

   return ret;
}

const dvfs_core *dvfs_unit_get_core(const dvfs_unit *unit, unsigned int id) {
   unsigned int i;

   assert(unit != NULL);

   for (i = 0; i < unit->nb_cores; i++) {
      if (unit->cores[i]->id == id) {
         return unit->cores[i];
      }
   }

   return NULL;
}

unsigned int dvfs_unit_get_freq(const dvfs_unit *unit) {
   unsigned int mfreq = 0, i;

   assert(unit != NULL);

   for (i = 0; i < unit->nb_cores; i++) {
      unsigned int cfreq = dvfs_core_get_current_freq(unit->cores[i]);
      mfreq = (mfreq >= cfreq ? mfreq : cfreq);
   }

   return mfreq;
}

unsigned int dvfs_unit_get_id (const dvfs_unit *unit) {
   assert (unit != NULL);
   return unit->id;
}
