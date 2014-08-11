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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libdvfs.h"

#define CHECK_ERROR(ctx,fct,message) { int result = fct; \
    if (result != DVFS_SUCCESS) { \
        printf(message" (%s).\n",dvfs_strerror(result)); \
        dvfs_stop(ctx); \
        return EXIT_FAILURE; \
    }}

int main(int argc, char **argv)
{
   unsigned int i;
   int id_result = DVFS_SUCCESS;

   (void) argc;
   (void) argv;

   dvfs_ctx *ctx = NULL;
   id_result = dvfs_start(&ctx,true);
   if (id_result != DVFS_SUCCESS)
   {
      perror ("DVFS Start");
      return EXIT_FAILURE;
   }

   const dvfs_core *core = NULL;
   CHECK_ERROR(ctx,dvfs_get_core(ctx, &core, 0),"Get core");

   unsigned int nb_freqs = 0;
   CHECK_ERROR(ctx,dvfs_core_get_nb_freqs (core,&nb_freqs),"Unable to get freq");

   for (i = 0; i < nb_freqs; i++) {
      printf("%u\n", core->freqs[i]);
   }
   const dvfs_unit *unit = NULL;
   CHECK_ERROR(ctx,dvfs_get_unit_by_core(ctx, core, &unit),"Get unit");

   CHECK_ERROR(ctx,dvfs_unit_set_gov(unit, "userspace"),"Unable to set governor");
   CHECK_ERROR(ctx,dvfs_unit_set_freq(unit, core->freqs[core->nb_freqs - 1]),"Unable to set freq");

   sleep(2);

   dvfs_stop(ctx);

   return EXIT_SUCCESS;
}
