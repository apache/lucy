/* Charmonizer/Probe/FuncMacro.h
 */

#ifndef H_CHAZ_FUNC_MACRO
#define H_CHAZ_FUNC_MACRO 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Run the FuncMacro module.
 * 
 * If __func__ successfully resolves, this will be defined:
 *
 * HAS_ISO_FUNC_MACRO
 * 
 * If __FUNCTION__ successfully resolves, this will be defined:
 * 
 * HAS_GNUC_FUNC_MACRO
 * 
 * If one or the other succeeds, these will be defined:
 * 
 * HAS_FUNC_MACRO
 * FUNC_MACRO
 * 
 * The "inline" keyword will also be probed for.  If it is available, the
 * following macro will be defined to "inline", otherwise it will be defined
 * to nothing.
 * 
 * INLINE
 */
void chaz_FuncMacro_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define FuncMacro_run    chaz_FuncMacro_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_FUNC_MACRO */



