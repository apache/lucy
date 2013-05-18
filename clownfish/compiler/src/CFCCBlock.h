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

/** Clownfish::CFC::Model::CBlock - A block of embedded C code.
 *
 * CBlock exists to support embedding literal C code within Clownfish header
 * files:
 *
 *     class Crustacean::Lobster {
 *         ...
 *
 *         inert inline void
 *         say_hello(Lobster *self);
 *     }
 *
 *     __C__
 *     #include <stdio.h>
 *     static CFISH_INLINE void
 *     crust_Lobster_say_hello(crust_Lobster *self)
 *     {
 *         printf("Prepare to die, human scum.\n");
 *     }
 *     __END_C__
 */

#ifndef H_CFCCBLOCK
#define H_CFCCBLOCK

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCCBlock CFCCBlock;

/** CBlock Constructor.
 *
 * @param contents The contents of the CBlock, not including delimiters.
 */
CFCCBlock*
CFCCBlock_new(const char *contents);

CFCCBlock*
CFCCBlock_init(CFCCBlock *self, const char *contents);

void
CFCCBlock_destroy(CFCCBlock *self);

/** Accessor.
 */
const char*
CFCCBlock_get_contents(CFCCBlock *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCCBLOCK */

