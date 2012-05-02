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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "module.h"

struct Animal {
    MetaClass *metaclass;
    void *superself;
};

struct Dog {
    MetaClass *metaclass;
    char *name;
    Animal superself;
};

struct Boxer {
    MetaClass *metaclass;
    Dog superself;
};

MetaClass *cANIMAL = NULL;
MetaClass *cDOG    = NULL;
MetaClass *cBOXER  = NULL;

uint64_t Animal_destroy_OFFSETS;
uint64_t Dog_destroy_OFFSETS;
uint64_t Boxer_destroy_OFFSETS;

uint64_t Animal_speak_OFFSETS;
uint64_t Dog_speak_OFFSETS;
uint64_t Boxer_speak_OFFSETS;

uint64_t Dog_ignore_name_OFFSETS;
uint64_t Boxer_ignore_name_OFFSETS;

uint64_t Boxer_drool_OFFSETS;

Animal*
Animal_init(Animal *self) {
    return self;
}

static void
S_Animal_speak(Animal *self) {
    (void)self;
    printf("*noise*\n");
}

Dog*
Dog_new(const char *name) {
    Dog *self = MetaClass_make_obj(cDOG);
    return Dog_init(self, name);
}

Dog*
Dog_init(Dog *self, const char *name) {
    Animal_init(&self->superself);
    self->name = strdup(name);
    return self;
}

static void
S_Dog_destroy(Dog *self) {
    free(self->name);
    Animal_destroy_t super_destroy =
        (Animal_destroy_t)MetaClass_method_pointer(cANIMAL, Animal_destroy_OFFSETS);
    void *view = (char*)&self->superself + (Animal_destroy_OFFSETS >> 32);
    super_destroy(view);
}

static void
S_Dog_speak(Dog *self) {
    (void)self;
    printf("Woof!\n");
}

static void
S_Dog_ignore_name(Dog *self) {
    printf("My name isn't \"%s\", it's \"cookie\".\n", self->name);
}

Boxer*
Boxer_new(const char *name) {
    Boxer *self = (Boxer*)MetaClass_make_obj(cBOXER);
    return Boxer_init(self, name);
}

Boxer*
Boxer_init(Boxer *self, const char *name) {
    Dog_init(&self->superself, name);
    return self;
}

static void
S_Boxer_drool(Dog *self) {
    (void)self;
    printf("*slobber*\n");
}

/**********************************************************************/

void
Module_bootstrap() {
    size_t num_methods = MetaClass_get_num_methods(cOBJECT);
    size_t base_size   = MetaClass_get_obj_alloc_size(cOBJECT) - sizeof(void*);

    // Animal
    num_methods += 1; // speak
    cANIMAL = MetaClass_new(cOBJECT, "Animal", sizeof(Animal) + base_size,
                            num_methods);
    Animal_destroy_OFFSETS
        = MetaClass_inherit_method(cANIMAL, Obj_destroy_OFFSETS);
    Animal_speak_OFFSETS
        = MetaClass_add_novel_method(cANIMAL, (method_t)S_Animal_speak);

    // Dog
    num_methods += 1; // ignore_name
    cDOG = MetaClass_new(cANIMAL, "Dog", sizeof(Dog) + base_size, num_methods);
    Dog_destroy_OFFSETS
        = MetaClass_override_method(cDOG, (method_t)S_Dog_destroy, Animal_destroy_OFFSETS);
    Dog_speak_OFFSETS
        = MetaClass_override_method(cDOG, (method_t)S_Dog_speak, Animal_speak_OFFSETS);
    Dog_ignore_name_OFFSETS
        = MetaClass_add_novel_method(cDOG, (method_t)S_Dog_ignore_name);

    // Boxer
    num_methods += 1; // drool
    cBOXER = MetaClass_new(cDOG, "Boxer", sizeof(Boxer) + base_size, num_methods);
    Boxer_destroy_OFFSETS
        = MetaClass_inherit_method(cBOXER, Dog_destroy_OFFSETS);
    Boxer_speak_OFFSETS
        = MetaClass_inherit_method(cBOXER, Dog_speak_OFFSETS);
    Boxer_ignore_name_OFFSETS
        = MetaClass_inherit_method(cBOXER, Dog_ignore_name_OFFSETS);
    Boxer_drool_OFFSETS
        = MetaClass_add_novel_method(cBOXER, (method_t)S_Boxer_drool);
}

void
Module_tear_down() {
    MetaClass_destroy(cBOXER);
    MetaClass_destroy(cDOG);
    MetaClass_destroy(cANIMAL);
}
