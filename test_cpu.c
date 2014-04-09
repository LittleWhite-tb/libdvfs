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

#include <stdio.h>
#include <unistd.h>

#include "libdvfs.h"

int main(int argc, char **argv)
{
   (void) argc;
   (void) argv;

   dvfs_ctx *ctx = dvfs_start(true);
   if (ctx == NULL) {
      perror ("DVFS Start");
      return -1;
   }

   if (dvfs_set_gov(ctx, "userspace") == 0) {
      perror ("Setting userspace");
      return -1;
   }
   
   if (dvfs_set_freq(ctx, 2200000) == 0) {
      perror ("Setting frequency");
      return -1;
   }

   sleep(2);

   dvfs_stop(ctx);
   
   if (dvfs_has_TB()) {
      printf("This CPU has TurboBoost\n");
   }

   return 0;
}
