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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "dso.h"

#define CPUFREQ    UINT64_C(2800000000)
#define NOINLINE   __attribute__ ((noinline))
uint64_t iterations;
#define ITERATIONS iterations

static inline method_t
Obj_Hello_PTR(obj_t *obj) {
    class_t *klass = obj->klass;
    return *(method_t*)((char*)klass + Obj_Hello_OFFSET);
}

static inline void
Obj_Hello(obj_t *obj) {
    class_t *klass = obj->klass;
    method_t method = *(method_t*)((char*)klass + Obj_Hello_OFFSET);
    method(obj);
}

static inline void
Obj_Hello_FIXED(obj_t *obj) {
    class_t *klass = obj->klass;
    method_t method = *(method_t*)((char*)klass + Obj_Hello_FIXED_OFFSET);
    method(obj);
}

void
loop_with_method_ptr(obj_t *obj) {
    method_t method = Obj_Hello_PTR(obj);

    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        method(obj);
    }
}

void
loop_with_wrapper(obj_t *obj) {
    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        Obj_Hello(obj);
    }
}

void
loop_with_fixed_offset_wrapper(obj_t *obj) {
    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        Obj_Hello_FIXED(obj);
    }
}

#ifdef HAS_ALIAS
void
loop_with_thunk(obj_t *obj) {
    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        Obj_Hello_THUNK(obj);
    }
}

void
loop_with_thunk_ptr(obj_t *obj) {
    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        Obj_Hello_THUNK_PTR(obj);
    }
}
#endif

NOINLINE void
single_call_with_wrapper(obj_t *obj) {
    Obj_Hello(obj);
}

void
call_with_wrapper(obj_t *obj) {
    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        single_call_with_wrapper(obj);
    }
}

#ifdef HAS_ALIAS
NOINLINE void
single_call_with_thunk(obj_t *obj) {
    Obj_Hello_THUNK(obj);
}

void
call_with_thunk_ptr(obj_t *obj) {
    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        single_call_with_thunk(obj);
    }
}

NOINLINE void
single_call_with_thunk_ptr(obj_t *obj) {
    Obj_Hello_THUNK_PTR(obj);
}

void
call_with_thunk(obj_t *obj) {
    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        single_call_with_thunk(obj);
    }
}
#endif

void
loop_with_simulated_inline(obj_t *obj) {
    for (uint64_t i = 0; i < ITERATIONS; ++i) {
        obj->value++;
    }
}

static void
bench(method_t fn, const char *name) {
    obj_t *obj = Obj_new();

    struct timeval t0;
    gettimeofday(&t0, NULL);

    fn(obj);

    struct timeval t1;
    gettimeofday(&t1, NULL);

    if (obj->value != ITERATIONS) {
        fprintf(stderr, "Unexpected obj->value: %" PRIu64 "\n", obj->value);
        abort();
    }

    uint64_t usec = (uint64_t)(t1.tv_sec - t0.tv_sec) * 1000000
                    + (t1.tv_usec - t0.tv_usec);
    printf("cycles/call with %s: %f\n", name,
           ((double)usec * CPUFREQ) / (1000000.0 * ITERATIONS));
}

int
main(int argc, char **argv) {
    if (argc > 1) {
        iterations = strtoll(argv[1], NULL, 10);
    }
    else {
        iterations = UINT64_C(1000000000);
    }
    bootstrap();

    bench(loop_with_method_ptr, "method ptr loop");
    bench(loop_with_wrapper, "wrapper loop");
    bench(loop_with_fixed_offset_wrapper, "fixed offset wrapper loop");
#ifdef HAS_ALIAS
    bench(loop_with_thunk, "thunk loop");
    bench(loop_with_thunk_ptr, "thunk ptr loop");
#endif
    bench(call_with_wrapper, "wrapper");
#ifdef HAS_ALIAS
    bench(call_with_thunk, "thunk");
    bench(call_with_thunk_ptr, "thunk ptr");
#endif
    bench(loop_with_simulated_inline, "simulated inline");

    return 0;
}

