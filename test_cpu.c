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
   (void) argc;
   (void) argv;

   int id_result=DVFS_SUCCESS;

   dvfs_ctx *ctx = NULL;
   id_result = dvfs_start(&ctx,true);
   if (id_result != DVFS_SUCCESS) {
      perror ("DVFS Start");
      return -1;
   }

   CHECK_ERROR(ctx,dvfs_set_gov(ctx, "userspace"),"Unable to set governor");
   CHECK_ERROR(ctx,dvfs_set_freq(ctx, 2200000),"Unable to set freq");

   sleep(2);

   dvfs_stop(ctx);

   if (dvfs_has_TB() == DVFS_TB_AVAILABLE) {
      printf("This CPU has TurboBoost\n");
   }

   return 0;
}
