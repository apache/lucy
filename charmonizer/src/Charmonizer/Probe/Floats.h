/* Charmonizer/Probe/Floats.h -- floating point types.
 * 
 * The following symbols will be created if the platform supports IEEE 754
 * floating point types:
 * 
 * F32_NAN
 * F32_INF
 * F32_NEGINF
 *
 * The following typedefs will be created if the platform supports IEEE 754
 * floating point types:
 * 
 * f32_t
 * f64_t
 *
 * Availability of the preceding typedefs is indicated by which of these are
 * defined:
 * 
 * HAS_F32_T
 * HAS_F64_T
 *
 * TODO: Actually test to see whether IEEE 754 is supported, rather than just
 * lying about it.
 */

#ifndef H_CHAZ_FLOATS
#define H_CHAZ_FLOATS

#ifdef __cplusplus
extern "C" {
#endif

/* Run the Floats module.
 */
void 
chaz_Floats_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define Floats_run    chaz_Floats_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_FLOATS */


/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

