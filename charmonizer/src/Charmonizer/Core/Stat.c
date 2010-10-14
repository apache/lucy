#define CHAZ_USE_SHORT_NAMES

#include <stdlib.h>
#include <string.h>

#include "Charmonizer/Core/Stat.h"

#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"
#include "Charmonizer/Core/Util.h"

static chaz_bool_t initialized    = false;
static chaz_bool_t stat_available = false;

/* Lazily compile _charm_stat. */
static void
S_init();

void
Stat_stat(const char *filepath, Stat *target)
{
    char *stat_output;
    size_t output_len;

    /* Failsafe. */
    target->valid = false;

    /* Lazy init. */
    if (!initialized) { S_init(); }

    /* Bail out if we didn't succeed in compiling/using _charm_stat. */
    if (!stat_available) { return; }

    /* Run _charm_stat. */
    Util_remove_and_verify("_charm_statout");
    OS_run_local("_charm_stat ", filepath, NULL);
    stat_output = Util_slurp_file("_charm_statout", &output_len);
    Util_remove_and_verify("_charm_statout");

    /* Parse the output of _charm_stat and store vars in Stat struct. */
    if (stat_output != NULL) {
        char *end_ptr = stat_output;
        target->size     = strtol(stat_output, &end_ptr, 10);
        stat_output      = end_ptr;
        target->blocks   = strtol(stat_output, &end_ptr, 10);
        target->valid = true;
    }

    return;
}

/* Source code for the _charm_stat utility. */
static char charm_stat_code[] = 
    QUOTE(  #include <stdio.h>                                     )
    QUOTE(  #include <sys/stat.h>                                  )
    QUOTE(  int main(int argc, char **argv) {                      )
    QUOTE(      FILE *out_fh = fopen("_charm_statout", "w+");      )
    QUOTE(      struct stat st;                                    )
    QUOTE(      if (argc != 2) { return 1; }                       )
    QUOTE(      if (stat(argv[1], &st) == -1) { return 2; }        )
    QUOTE(      fprintf(out_fh, "%ld ", (long)st.st_size);         )
    QUOTE(      fprintf(out_fh, "%ld\n", (long)st.st_blocks);      )
    QUOTE(      return 0;                                          )
    QUOTE(  }                                                      );

static void
S_init()
{
    /* Only try this once. */
    initialized = true;
    if (Util_verbosity) {
        printf("Attempting to compile _charm_stat utility...\n");
    }

    /* Bail if sys/stat.h isn't available. */
    if (!HeadCheck_check_header("sys/stat.h")) { return; }


    /* If the compile succeeds, open up for business. */
    stat_available = CC_compile_exe("_charm_stat.c", "_charm_stat", 
        charm_stat_code, strlen(charm_stat_code));
    remove("_charm_stat.c");
}


/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

