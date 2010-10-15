/* Charmonizer/Probe/VariadicMacros.h
 */

#ifndef H_CHAZ_VARIADIC_MACROS
#define H_CHAZ_VARIADIC_MACROS 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Run the VariadicMacros module.
 *
 * If your compiler supports ISO-style variadic macros, this will be defined:
 * 
 * HAS_ISO_VARIADIC_MACROS
 * 
 * If your compiler supports GNU-style variadic macros, this will be defined:
 * 
 * HAS_GNUC_VARIADIC_MACROS
 * 
 * If you have at least one of the above, this will be defined:
 * 
 * HAS_VARIADIC_MACROS
 */
void chaz_VariadicMacros_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define VariadicMacros_run    chaz_VariadicMacros_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_VARIADIC_MACROS */



