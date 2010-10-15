/* Charmonizer/Probe/AtomicOps.h
 */

#ifndef H_CHAZ_ATOMICOPS
#define H_CHAZ_ATOMICOPS 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Run the AtomicOps module.
 *
 * These following symbols will be defined if the associated headers are
 * available:
 * 
 * HAS_LIBKERN_OSATOMIC_H  <libkern/OSAtomic.h> (Mac OS X)
 * HAS_SYS_ATOMIC_H        <sys/atomic.h>       (Solaris)
 * HAS_INTRIN_H            <intrin.h>           (Windows)
 *
 * This symbol is defined if OSAtomicCompareAndSwapPtr is available:
 *
 * HAS_OSATOMIC_CAS_PTR
 */
void chaz_AtomicOps_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define AtomicOps_run    chaz_AtomicOps_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_ATOMICOPS */



