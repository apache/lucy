/* Charmonizer/Probe/LargeFiles.h
 */

#ifndef H_CHAZ_LARGE_FILES
#define H_CHAZ_LARGE_FILES

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* The LargeFiles module attempts to detect these symbols or alias them to
 * synonyms:
 * 
 * off64_t
 * fopen64
 * ftello64
 * fseeko64
 * 
 * If the attempt succeeds, this will be defined:
 * 
 * HAS_LARGE_FILE_SUPPORT
 *
 * Additionally, 64-bit versions of lseek and pread may be detected/aliased:
 *
 * lseek64
 * pread64
 * 
 * Use of the off64_t symbol may require sys/types.h.
 */
void chaz_LargeFiles_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define LargeFiles_run    chaz_LargeFiles_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_LARGE_FILES */

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

