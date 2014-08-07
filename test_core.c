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
#include <unistd.h>

#include "libdvfs.h"

int main(int argc, char **argv)
{
   unsigned int i;
   int id_result = DVFS_SUCCESS;

   (void) argc;
   (void) argv;

   dvfs_ctx *ctx = NULL;
   id_result = dvfs_start(&ctx,true);
   if (ctx == NULL)
   {
      perror ("DVFS Start");
      return -1;
   }

   const dvfs_core *core = NULL;
   id_result = dvfs_get_core(ctx, &core, 0);
   if (id_result != DVFS_SUCCESS) {
      perror ("Get core");
      return -1;
   }

   unsigned int nb_freqs = 0;
   id_result = dvfs_core_get_nb_freqs (core,&nb_freqs);
   if ( id_result != DVFS_SUCCESS )
   {
      fprintf(stderr,"Unable to get freq : %s",dvfs_strerror(id_result));
      perror ("Perror : ");
      dvfs_stop(ctx);
      return -1;
   }

   for (i = 0; i < nb_freqs; i++) {
      printf("%u\n", core->freqs[i]);
   }
   const dvfs_unit *unit = NULL;
   id_result = dvfs_get_unit(ctx, core, &unit);
   if (id_result != DVFS_SUCCESS) {
      perror ("get unit");
      dvfs_stop(ctx);
      return -1;
   }

   id_result = dvfs_unit_set_gov(unit, "userspace");
   if (id_result != DVFS_SUCCESS) {
      fprintf(stderr,"Unable to set governort : %s\n",dvfs_strerror(id_result));
      perror ("Set governor");
      dvfs_stop(ctx);
      return -1;
   }

   id_result = dvfs_unit_set_freq(unit, core->freqs[core->nb_freqs - 1]);
   if (id_result != DVFS_SUCCESS) {
      fprintf(stderr,"Unable to set freq : %s\n",dvfs_strerror(id_result));
      perror ("Unit set freq");
   }

   sleep(2);

   dvfs_stop(ctx);

   return 0;
}
