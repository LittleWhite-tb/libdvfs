#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdvfs.h"

int main(int argc, char **argv) {
   unsigned int i, j;
   int coreId = -1;

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
   dvfs_ctx *ctx = dvfs_start();

   if (ctx == NULL) {
      printf("Failed to read topology information.\n");
      return EXIT_FAILURE;
   }

   // print all the cores
   if (coreId == -1) {
      for (i = 0; i < ctx->nb_units; i++) {
         for (j = 0; j < ctx->units[i]->nb_cores; j++) {
            printf("%u ", ctx->units[i]->cores[j]->id);
         }

         if (i < ctx->nb_units - 1) {
            printf("| ");
         }
      }
      printf("\n");
   // one specific core requested
   } else {
      const dvfs_core *core = dvfs_get_core(ctx, coreId);

      if (core == NULL) {
         dvfs_stop(ctx);
         printf("Invalid core number provided\n");
         return EXIT_FAILURE;
      }

      const dvfs_unit *unit = dvfs_get_unit(ctx, core);
      for (i = 0; i < unit->nb_cores; i++) {
         printf("%u ", unit->cores[i]->id);
      }
      printf("\n");
   }
   
   dvfs_stop(ctx);

   return EXIT_SUCCESS;
}
