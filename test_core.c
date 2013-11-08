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

   dvfs_core *ctx = dvfs_core_open(0);
   if ( ctx == NULL )
   {
      return -1;
   }

   for (i = 0; i < ctx->nb_freqs; i++) {
      printf("%u\n", ctx->freqs[i]);
   }
   
   dvfs_core_set_gov(ctx, "userspace");
   dvfs_core_set_freq(ctx, ctx->freqs[ctx->nb_freqs - 1]);

   sleep(5);

   dvfs_core_close(ctx);
   
   return 0;
}
