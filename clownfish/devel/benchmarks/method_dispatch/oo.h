#ifndef OO_H
#define OO_H

#include <stddef.h>
#include <stdint.h>

typedef struct class_t class_t;

typedef struct obj_t {
    size_t    refcount;
    class_t  *klass;
    uint64_t  value;
} obj_t;

typedef void (*method_t)(obj_t *obj);

struct class_t {
    char     *name;
    size_t    class_size;
    method_t  vtable[1];
};

#endif /* OO_H */

