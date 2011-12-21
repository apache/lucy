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

/** Clownfish::CFC::Dumpable - Auto-generate code for "dumpable" classes.
 *
 * If a class declares that it has the attribute "dumpable", but does not
 * declare either Dump or Load(), Clownfish::CFC::Dumpable will attempt to
 * auto-generate those methods if methods inherited from the parent class do
 * not suffice.
 * 
 *     class Foo::Bar inherits Foo : dumpable {
 *         Thing *thing;
 * 
 *         public inert incremented Bar*
 *         new();
 * 
 *         void
 *         Destroy(Bar *self);
 *     }
 */

#ifndef H_CFCDUMPABLE
#define H_CFCDUMPABLE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCDumpable CFCDumpable;
struct CFCClass;

/** Constructor. 
 */
CFCDumpable*
CFCDumpable_new(void);

CFCDumpable*
CFCDumpable_init(CFCDumpable *self);

void
CFCDumpable_destroy(CFCDumpable *self);


/** Analyze a class with the attribute "dumpable" and add Dump() or Load()
 * methods as necessary.
 */
void
CFCDumpable_add_dumpables(CFCDumpable *self, struct CFCClass *klass);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCDUMPABLE */

