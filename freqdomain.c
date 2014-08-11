#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdvfs.h"

#define CHECK_ERROR(ctx,fct,message) { int result = fct; \
    if (result != DVFS_SUCCESS) { \
        printf(message" (%s).\n",dvfs_strerror(result)); \
        dvfs_stop(ctx); \
        return EXIT_FAILURE; \
    }}

int main(int argc, char **argv) {
   unsigned int i, j;
   int coreId = -1;
   int result = DVFS_SUCCESS;

   // parse the arguments
   if (argc > 1) {
      if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
         printf("Determines the cpu cores on the same frequency domain\n\n");
         printf("\nUsage: %s [core_id]\n", argv[0]);
         printf("If core_id is provided, outputs the cores on the same frequency domain (including core_id).\n\n");
         printf("Otherwise, all the cores identifiers are printed, grouped by frequency domain. The groups are separated by the character '|'.\n\n");
         printf("For instance if core 0 and 1 lie in the same frequency domain, while cores 2 and 3 lie in a different frequency domain, the output will be\n");
         printf("0 1 | 2 3 \n");
         return EXIT_SUCCESS;
      }

      coreId = strtol(argv[1], NULL, 10);
      if (errno == EINVAL) {
         printf("Invalid core id provided\n");
         return EXIT_FAILURE;
      }
   }

   // read the topology
   dvfs_ctx *ctx = NULL;
   result = dvfs_start(&ctx,true);

   if (result != DVFS_SUCCESS) {
      printf("Failed to read topology information (%s).\n",dvfs_strerror(result));
      return EXIT_FAILURE;
   }

   // print all the cores
   if (coreId == -1)
   {
      unsigned int nb_units = 0;
      CHECK_ERROR(ctx,dvfs_get_nb_unit(ctx,&nb_units),"Failed to get number of DVFS units in context");

      for (i = 0; i < nb_units; i++)
      {
         unsigned int nb_cores = 0;
         const dvfs_unit* unit = NULL;
         CHECK_ERROR(ctx,dvfs_get_unit_by_id(ctx,&unit,i),"Failed to get DVFS unit");
         CHECK_ERROR(ctx,dvfs_unit_get_nb_core(unit,&nb_cores),"Failed to get number of DVFS cores in unit");

         for (j = 0; j < nb_cores; j++)
         {
            unsigned int id=0;
            dvfs_core* core = NULL;
            CHECK_ERROR(ctx,dvfs_unit_get_core(unit,&core,j),"Failed to get DVFS core");
            CHECK_ERROR(ctx,dvfs_core_get_id(core,&id),"Failed to get number of ID in DVFS core");

            printf("%u ", id);
         }

         if (i < nb_units - 1) {
            printf("| ");
         }
      }
      printf("\n");
   // one specific core requested
   } else {
      const dvfs_core *core = NULL;
      CHECK_ERROR(ctx,dvfs_get_core(ctx, &core, coreId),"Invalid core number provided");

      const dvfs_unit *unit = NULL;
      CHECK_ERROR(ctx,dvfs_get_unit_by_core(ctx, core, &unit),"");

      unsigned int nb_cores = 0;
      CHECK_ERROR(ctx,dvfs_unit_get_nb_core(unit,&nb_cores),"Failed to get number of DVFS cores in unit");

      for (i = 0; i < nb_cores; i++)
      {
         unsigned int id=0;
         CHECK_ERROR(ctx,dvfs_core_get_id(unit->cores[i],&id),"Failed to get number of ID in DVFS core");

         printf("%u ", id);
      }
      printf("\n");
   }

   dvfs_stop(ctx);

   return EXIT_SUCCESS;
}
