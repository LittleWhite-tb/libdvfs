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
   unsigned int i;

   (void) argc;
   (void) argv;

   dvfs_ctx *ctx = dvfs_start();
   if ( ctx == NULL )
   {
      return -1;
   }

   const dvfs_core *core = dvfs_get_core(ctx, 0);
   for (i = 0; i < core->nb_freqs; i++) {
      printf("%u\n", core->freqs[i]);
   }
   const dvfs_unit *unit = dvfs_get_unit(ctx, core);
   
   dvfs_unit_set_gov(unit, "userspace");
   dvfs_unit_set_freq(unit, core->freqs[core->nb_freqs - 1]);

   sleep(5);

   dvfs_stop(ctx);
   
   return 0;
}
