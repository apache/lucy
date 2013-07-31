#include <stdio.h>
#include <stdlib.h>

#include "dso.h"

static void Obj_hello(obj_t *obj);

void thunk3(obj_t *obj);

class_t *OBJ;
size_t Obj_Hello_OFFSET;
method_t Obj_Hello_THUNK_PTR;

void
bootstrap() {
    size_t method_idx = 3;
    size_t class_size = offsetof(class_t, vtable)
                        + (method_idx + 1) * sizeof(method_t);

    OBJ = (class_t*)calloc(1, class_size);

    OBJ->name       = "Obj";
    OBJ->class_size = class_size;

    Obj_Hello_OFFSET = offsetof(class_t, vtable)
                       + method_idx * sizeof(method_t);
    OBJ->vtable[method_idx] = Obj_hello;
    Obj_Hello_THUNK_PTR = thunk3;
}

obj_t*
Obj_new() {
    obj_t *self = (obj_t *)malloc(sizeof(obj_t));

    self->refcount = 1;
    self->klass    = OBJ;
    self->value    = 0;

    return self;
}

static void
Obj_hello(obj_t *obj) {
    ++obj->value;
}

void
thunk3(obj_t *obj) {
    obj->klass->vtable[3](obj);
}

