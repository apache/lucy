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

#ifndef MODULE_H
#define MODULE_H

#include "core.h"

typedef struct Animal Animal;
typedef struct Dog    Dog;
typedef struct Boxer  Boxer;

extern MetaClass *cANIMAL;
extern MetaClass *cDOG;
extern MetaClass *cBOXER;

void
Module_bootstrap(void);

void
Module_tear_down(void);

Animal*
Animal_init(Animal *self);

typedef void 
(*Animal_destroy_t)(Animal *self);
extern uint64_t Animal_destroy_OFFSETS;
static inline void
Animal_destroy(Animal *self) {
    const uint64_t offsets = Animal_destroy_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Animal_destroy_t method = *((Animal_destroy_t*)method_address);
    method(view);
}

typedef void 
(*Animal_speak_t)(Animal *self);
extern uint64_t Animal_speak_OFFSETS;
static inline void
Animal_speak(Animal *self) {
    const uint64_t offsets = Animal_speak_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Animal_speak_t method = *((Animal_speak_t*)method_address);
    method(view);
}

Dog*
Dog_new(const char *name);

Dog*
Dog_init(Dog *self, const char *name);

typedef void 
(*Dog_destroy_t)(Dog *self);
extern uint64_t Dog_destroy_OFFSETS;
static inline void
Dog_destroy(Dog *self) {
    const uint64_t offsets = Dog_destroy_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Dog_destroy_t method = *((Dog_destroy_t*)method_address);
    method(view);
}

static inline void
Dog_destroy_NEXT(Dog *self) {
    const uint64_t offsets = Dog_destroy_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Dog_destroy_t method = *((Dog_destroy_t*)method_address);
    method(view);
}

typedef void 
(*Dog_speak_t)(Dog *self);
extern uint64_t Dog_speak_OFFSETS;
static inline void
Dog_speak(Dog *self) {
    const uint64_t offsets = Dog_speak_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Dog_speak_t method = *((Dog_speak_t*)method_address);
    method(view);
}

typedef void 
(*Dog_ignore_name_t)(Dog *self);
extern uint64_t Dog_ignore_name_OFFSETS;
static inline void
Dog_ignore_name(Dog *self) {
    const uint64_t offsets = Dog_ignore_name_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Dog_ignore_name_t method = *((Dog_ignore_name_t*)method_address);
    method(view);
}

Boxer*
Boxer_new(const char *name);

Boxer*
Boxer_init(Boxer *self, const char *name);

typedef void 
(*Boxer_destroy_t)(Boxer *self);
extern uint64_t Boxer_destroy_OFFSETS;
static inline void
Boxer_destroy(Boxer *self) {
    const uint64_t offsets = Boxer_destroy_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Boxer_destroy_t method = *((Boxer_destroy_t*)method_address);
    method(view);
}

typedef void 
(*Boxer_speak_t)(Boxer *self);
extern uint64_t Boxer_speak_OFFSETS;
static inline void
Boxer_speak(Boxer *self) {
    const uint64_t offsets = Boxer_speak_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Boxer_speak_t method = *((Boxer_speak_t*)method_address);
    method(view);
}

typedef void 
(*Boxer_ignore_name_t)(Boxer *self);
extern uint64_t Boxer_ignore_name_OFFSETS;
static inline void
Boxer_ignore_name(Boxer *self) {
    const uint64_t offsets = Boxer_ignore_name_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Boxer_ignore_name_t method = *((Boxer_ignore_name_t*)method_address);
    method(view);
}

typedef void 
(*Boxer_drool_t)(Boxer *self);
extern uint64_t Boxer_drool_OFFSETS;
static inline void
Boxer_drool(Boxer *self) {
    const uint64_t offsets = Boxer_drool_OFFSETS;
    void *const view = (char*)self + (int32_t)(offsets >> 32);
    char *const method_address = *(char**)self + (uint32_t)offsets;
    Boxer_drool_t method = *((Boxer_drool_t*)method_address);
    method(view);
}

#endif /* MODULE_H */
