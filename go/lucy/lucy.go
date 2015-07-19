/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package lucy

/*
#define C_LUCY_REGEXTOKENIZER

#include "lucy_parcel.h"
#include "Lucy/Analysis/RegexTokenizer.h"

extern lucy_RegexTokenizer*
GOLUCY_RegexTokenizer_init(lucy_RegexTokenizer *self, cfish_String *pattern);
extern lucy_RegexTokenizer*
(*GOLUCY_RegexTokenizer_init_BRIDGE)(lucy_RegexTokenizer *self,
									 cfish_String *pattern);
extern void
GOLUCY_RegexTokenizer_Destroy(lucy_RegexTokenizer *self);
extern void
(*GOLUCY_RegexTokenizer_Destroy_BRIDGE)(lucy_RegexTokenizer *self);
extern void
GOLUCY_RegexTokenizer_Tokenize_Utf8(lucy_RegexTokenizer *self, char *str,
									size_t string_len, lucy_Inversion *inversion);
extern void
(*GOLUCY_RegexTokenizer_Tokenize_Utf8_BRIDGE)(lucy_RegexTokenizer *self, const char *str,
											  size_t string_len, lucy_Inversion *inversion);


// C symbols linked into a Go-built package archive are not visible to
// external C code -- but internal code *can* see symbols from outside.
// This allows us to fake up symbol export by assigning values only known
// interally to external symbols during Go package initialization.
static CFISH_INLINE void
GOLUCY_glue_exported_symbols() {
	GOLUCY_RegexTokenizer_init_BRIDGE = GOLUCY_RegexTokenizer_init;
	GOLUCY_RegexTokenizer_Destroy_BRIDGE = GOLUCY_RegexTokenizer_Destroy;
	GOLUCY_RegexTokenizer_Tokenize_Utf8_BRIDGE
		= (LUCY_RegexTokenizer_Tokenize_Utf8_t)GOLUCY_RegexTokenizer_Tokenize_Utf8;
}

*/
import "C"
import _ "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func init() {
	C.GOLUCY_glue_exported_symbols()
	C.lucy_bootstrap_parcel()
}

//export GOLUCY_RegexTokenizer_init
func GOLUCY_RegexTokenizer_init(rt *C.lucy_RegexTokenizer, pattern *C.cfish_String) *C.lucy_RegexTokenizer {
	return nil
}

//export GOLUCY_RegexTokenizer_Destroy
func GOLUCY_RegexTokenizer_Destroy(rt *C.lucy_RegexTokenizer) {
}

//export GOLUCY_RegexTokenizer_Tokenize_Utf8
func GOLUCY_RegexTokenizer_Tokenize_Utf8(rt *C.lucy_RegexTokenizer, str *C.char,
	stringLen C.size_t, inversion *C.lucy_Inversion) {
}
