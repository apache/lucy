#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/UnusedVars.h"
#include <string.h>
#include <stdio.h>


void
UnusedVars_run(void) 
{
    ConfWriter_start_module("UnusedVars");
    
    /* Write the macros (no test, these are the same everywhere). */
    ConfWriter_append_conf("#define CHY_UNUSED_VAR(x) ((void)x)\n");
    ConfWriter_append_conf("#define CHY_UNREACHABLE_RETURN(type) return (type)0\n");

    /* Shorten. */
    ConfWriter_start_short_names();
    ConfWriter_shorten_macro("UNUSED_VAR");
    ConfWriter_shorten_macro("UNREACHABLE_RETURN");
    ConfWriter_end_short_names();

    ConfWriter_end_module();
}


/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

