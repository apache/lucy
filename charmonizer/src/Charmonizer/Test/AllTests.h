/* Charmonizer/Test.h - test Charmonizer's output.
 */

#ifndef H_CHAZ_ALL_TESTS
#define H_CHAZ_ALL_TESTS

#ifdef __cplusplus
extern "C" {
#endif

#include "Charmonizer/Test.h"

/* Initialize the AllTests module.
 */
void
chaz_AllTests_init();

/* Run all tests.
 */
void
chaz_AllTests_run();

/* These tests all require the file charmony.h.
 * 
 * Since Charmonizer conditionally defines many symbols, it can be difficult
 * to tell whether a symbol is missing because it should not have been
 * generated, or whether it is missing because an error occurred.  These test
 * functions make the assumption that any missing symbols have a good excuse
 * for their absence, and test only defined symbols.  This may result in
 * undetected failure some of the time.  However, missing symbols required by
 * your application will trigger compile-time errors, so the theoretical
 * problem of silent failure is less severe than it appears, affecting only
 * fallbacks.
 */

chaz_TestBatch* 
chaz_TDirManip_prepare();

chaz_TestBatch* 
chaz_TFuncMacro_prepare();

chaz_TestBatch* 
chaz_THeaders_prepare();

chaz_TestBatch* 
chaz_TIntegers_prepare();

chaz_TestBatch* 
chaz_TLargeFiles_prepare();

chaz_TestBatch* 
chaz_TUnusedVars_prepare();

chaz_TestBatch* 
chaz_TVariadicMacros_prepare();

void
chaz_TDirManip_run(chaz_TestBatch *batch);

void
chaz_TFuncMacro_run(chaz_TestBatch *batch);

void
chaz_THeaders_run(chaz_TestBatch *batch);

void
chaz_TIntegers_run(chaz_TestBatch *batch);

void
chaz_TLargeFiles_run(chaz_TestBatch *batch);

void
chaz_TUnusedVars_run(chaz_TestBatch *batch);

void
chaz_TVariadicMacros_run(chaz_TestBatch *batch);

#ifdef CHAZ_USE_SHORT_NAMES
  #define TDirManip_prepare            chaz_TDirManip_prepare
  #define TDirManip_run                chaz_TDirManip_run
  #define TFuncMacro_prepare           chaz_TFuncMacro_prepare
  #define THeaders_prepare             chaz_THeaders_prepare
  #define TIntegers_prepare            chaz_TIntegers_prepare
  #define TLargeFiles_prepare          chaz_TLargeFiles_prepare
  #define TUnusedVars_prepare          chaz_TUnusedVars_prepare
  #define TVariadicMacros_prepare      chaz_TVariadicMacros_prepare
  #define TFuncMacro_run               chaz_TFuncMacro_run
  #define THeaders_run                 chaz_THeaders_run
  #define TIntegers_run                chaz_TIntegers_run
  #define TLargeFiles_run              chaz_TLargeFiles_run
  #define TUnusedVars_run              chaz_TUnusedVars_run
  #define TVariadicMacros_run          chaz_TVariadicMacros_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_TEST */


/**
 * Copyright 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

