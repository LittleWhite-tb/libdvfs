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
   (void) argc;
   (void) argv;

   int id_result=DVFS_SUCCESS;

   dvfs_ctx *ctx = NULL;
   id_result = dvfs_start(&ctx,true);
   if (id_result != DVFS_SUCCESS) {
      perror ("DVFS Start");
      return -1;
   }

   id_result = dvfs_set_gov(ctx, "userspace");
   if (id_result != DVFS_SUCCESS)
   {
      fprintf(stderr,"Unable to set governor : %s (%d)\n",dvfs_strerror(id_result),id_result);
      perror ("Setting userspace");
      dvfs_stop(ctx);
      return -1;
   }

   id_result = dvfs_set_freq(ctx, 2200000);
   if (id_result != DVFS_SUCCESS)
   {
      fprintf(stderr,"Unable to set freq : %s\n",dvfs_strerror(id_result));
      perror ("Setting frequency");
      dvfs_stop(ctx);
      return -1;
   }

   sleep(2);

   dvfs_stop(ctx);

   if (dvfs_has_TB() == 1) {
      printf("This CPU has TurboBoost\n");
   }

   return 0;
}
