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

#include "dvfs_unit.h"

#include <assert.h>
#include <stdlib.h>

dvfs_unit *dvfs_unit_open(unsigned int nb_cores, dvfs_core **cores) {
   assert(cores != NULL);

   dvfs_unit *res = malloc(sizeof(*res));
   res->nb_cores = nb_cores;
   res->cores = cores;
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

void dvfs_unit_set_gov(const dvfs_unit *unit, const char *gov) {
   unsigned int i;

   assert(unit != NULL);

   for (i = 0; i < unit->nb_cores; i++) {
      dvfs_core_set_gov(unit->cores[i], gov);
   }
}

void dvfs_unit_set_freq(dvfs_unit *unit, unsigned int freq) {
   unsigned int i;

   assert(unit != NULL);

   for (i = 0; i < unit->nb_cores; i++) {
      dvfs_core_set_freq(unit->cores[i], freq);
   }
}

bool dvfs_unit_contains(dvfs_unit *unit, unsigned int id) {
   unsigned int i;

   assert(unit != NULL);

   for (i = 0; i < unit->nb_cores; i++) {
      if (unit->cores[i]->id == id) {
         return true;
      }
   }

   return false;
}

