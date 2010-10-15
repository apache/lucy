/* Charmonizer/Probe/UnusedVars.h
 */

#ifndef H_CHAZ_UNUSED_VARS
#define H_CHAZ_UNUSED_VARS 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Run the UnusedVars module.
 *
 * These symbols are exported:
 * 
 * UNUSED_VAR(var)
 * UNREACHABLE_RETURN(type)
 * 
 */
void chaz_UnusedVars_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define UnusedVars_run    chaz_UnusedVars_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_UNUSED_VARS */



