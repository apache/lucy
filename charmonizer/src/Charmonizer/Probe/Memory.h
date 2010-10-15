/* Charmonizer/Probe/Memory.h
 */

#ifndef H_CHAZ_MEMORY
#define H_CHAZ_MEMORY 

#ifdef __cplusplus
extern "C" {
#endif

/* The Memory module attempts to detect these symbols or alias them to
 * synonyms:
 * 
 * alloca
 *
 * These following symbols will be defined if the associated headers are
 * available:
 * 
 * HAS_ALLOCA_H            <alloca.h> 
 * HAS_MALLOC_H            <malloc.h>
 *
 * Defined if alloca() is available via stdlib.h:
 *
 * ALLOCA_IN_STDLIB_H
 */
void chaz_Memory_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define Memory_run    chaz_Memory_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_MEMORY */



