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

#define C_LUCY_SORTEXTERNAL
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Util/SortExternal.h"

// Refill the main cache, drawing from the caches of all runs.
static void
S_refill_cache(SortExternal *self);

// Absorb all the items which are "in-range" from all the Runs into the main
// cache.
static void
S_absorb_slices(SortExternal *self, uint8_t *endpost);

// Return the address for the item in one of the runs' caches which is the
// highest in sort order, but which we can guarantee is lower in sort order
// than any item which has yet to enter a run cache.
static uint8_t*
S_find_endpost(SortExternal *self);

// Determine how many cache items are less than or equal to [endpost].
static uint32_t
S_find_slice_size(SortExternal *self, uint8_t *endpost);

SortExternal*
SortEx_init(SortExternal *self, size_t width) {
    // Assign.
    self->width        = width;

    // Init.
    self->mem_thresh   = U32_MAX;
    self->cache        = NULL;
    self->cache_cap    = 0;
    self->cache_max    = 0;
    self->cache_tick   = 0;
    self->scratch      = NULL;
    self->scratch_cap  = 0;
    self->runs         = VA_new(0);
    self->slice_sizes  = NULL;
    self->slice_starts = NULL;
    self->num_slices   = 0;
    self->flipped      = false;

    ABSTRACT_CLASS_CHECK(self, SORTEXTERNAL);
    return self;
}

void
SortEx_destroy(SortExternal *self) {
    FREEMEM(self->scratch);
    FREEMEM(self->slice_sizes);
    FREEMEM(self->slice_starts);
    if (self->cache) {
        SortEx_Clear_Cache(self);
        FREEMEM(self->cache);
    }
    DECREF(self->runs);
    SUPER_DESTROY(self, SORTEXTERNAL);
}

void
SortEx_clear_cache(SortExternal *self) {
    self->cache_max    = 0;
    self->cache_tick   = 0;
}

void
SortEx_feed(SortExternal *self, void *data) {
    const size_t width = self->width;
    if (self->cache_max == self->cache_cap) {
        size_t amount = Memory_oversize(self->cache_max + 1, width);
        SortEx_Grow_Cache(self, amount);
    }
    uint8_t *target = self->cache + self->cache_max * width;
    memcpy(target, data, width);
    self->cache_max++;
}

static INLINE void*
SI_peek(SortExternal *self) {
    if (self->cache_tick >= self->cache_max) {
        S_refill_cache(self);
    }

    if (self->cache_max > 0) {
        return self->cache + self->cache_tick * self->width;
    }
    else {
        return NULL;
    }
}

void*
SortEx_fetch(SortExternal *self) {
    void *address = SI_peek(self);
    self->cache_tick++;
    return address;
}

void*
SortEx_peek(SortExternal *self) {
    return SI_peek(self);
}

void
SortEx_sort_cache(SortExternal *self) {
    if (self->cache_tick != 0) {
        THROW(ERR, "Cant Sort_Cache() after fetching %u32 items", self->cache_tick);
    }
    if (self->cache_max != 0) {
        VTable *vtable = SortEx_Get_VTable(self);
        lucy_Sort_compare_t compare
            = (lucy_Sort_compare_t)METHOD(vtable, SortEx, Compare);
        if (self->scratch_cap < self->cache_cap) {
            self->scratch_cap = self->cache_cap;
            self->scratch = (uint8_t*)REALLOCATE(
                                self->scratch,
                                self->scratch_cap * self->width);
        }
        Sort_mergesort(self->cache, self->scratch, self->cache_max,
                       self->width, compare, self);
    }
}

void
SortEx_flip(SortExternal *self) {
    SortEx_Flush(self);
    self->flipped = true;
}

void
SortEx_add_run(SortExternal *self, SortExternal *run) {
    VA_Push(self->runs, (Obj*)run);
    uint32_t num_runs = VA_Get_Size(self->runs);
    self->slice_sizes = (uint32_t*)REALLOCATE(
                            self->slice_sizes,
                            num_runs * sizeof(uint32_t));
    self->slice_starts = (uint8_t**)REALLOCATE(
                             self->slice_starts,
                             num_runs * sizeof(uint8_t*));
}

static void
S_refill_cache(SortExternal *self) {
    // Reset cache vars.
    SortEx_Clear_Cache(self);

    // Make sure all runs have at least one item in the cache.
    uint32_t i = 0;
    while (i < VA_Get_Size(self->runs)) {
        SortExternal *const run = (SortExternal*)VA_Fetch(self->runs, i);
        if (SortEx_Cache_Count(run) > 0 || SortEx_Refill(run) > 0) {
            i++; // Run has some elements, so keep.
        }
        else {
            VA_Excise(self->runs, i, 1);
        }
    }

    // Absorb as many elems as possible from all runs into main cache.
    if (VA_Get_Size(self->runs)) {
        uint8_t *endpost = S_find_endpost(self);
        S_absorb_slices(self, endpost);
    }
}

static uint8_t*
S_find_endpost(SortExternal *self) {
    uint8_t *endpost = NULL;
    const size_t width = self->width;

    for (uint32_t i = 0, max = VA_Get_Size(self->runs); i < max; i++) {
        // Get a run and retrieve the last item in its cache.
        SortExternal *const run = (SortExternal*)VA_Fetch(self->runs, i);
        const uint32_t tick = run->cache_max - 1;
        if (tick >= run->cache_cap || run->cache_max < 1) {
            THROW(ERR, "Invalid SortExternal cache access: %u32 %u32 %u32", tick,
                  run->cache_max, run->cache_cap);
        }
        else {
            // Cache item with the highest sort value currently held in memory
            // by the run.
            uint8_t *candidate = run->cache + tick * width;

            // If it's the first run, item is automatically the new endpost.
            if (i == 0) {
                endpost = candidate;
            }
            // If it's less than the current endpost, it's the new endpost.
            else if (SortEx_Compare(self, candidate, endpost) < 0) {
                endpost = candidate;
            }
        }
    }

    return endpost;
}

static void
S_absorb_slices(SortExternal *self, uint8_t *endpost) {
    size_t      width        = self->width;
    uint32_t    num_runs     = VA_Get_Size(self->runs);
    uint8_t   **slice_starts = self->slice_starts;
    uint32_t   *slice_sizes  = self->slice_sizes;
    VTable     *vtable       = SortEx_Get_VTable(self);
    lucy_Sort_compare_t compare
        = (lucy_Sort_compare_t)METHOD(vtable, SortEx, Compare);

    if (self->cache_max != 0) { THROW(ERR, "Can't refill unless empty"); }

    // Move all the elements in range into the main cache as slices.
    for (uint32_t i = 0; i < num_runs; i++) {
        SortExternal *const run = (SortExternal*)VA_Fetch(self->runs, i);
        uint32_t slice_size = S_find_slice_size(run, endpost);

        if (slice_size) {
            // Move slice content from run cache to main cache.
            if (self->cache_max + slice_size > self->cache_cap) {
                size_t cap = Memory_oversize(self->cache_max + slice_size,
                                             width);
                SortEx_Grow_Cache(self, cap);
            }
            memcpy(self->cache + self->cache_max * width,
                   run->cache + run->cache_tick * width,
                   slice_size * width);
            run->cache_tick += slice_size;
            self->cache_max += slice_size;

            // Track number of slices and slice sizes.
            slice_sizes[self->num_slices++] = slice_size;
        }
    }

    // Transform slice starts from ticks to pointers.
    uint32_t total = 0;
    for (uint32_t i = 0; i < self->num_slices; i++) {
        slice_starts[i] = self->cache + total * width;
        total += slice_sizes[i];
    }

    // The main cache now consists of several slices.  Sort the main cache,
    // but exploit the fact that each slice is already sorted.
    if (self->scratch_cap < self->cache_cap) {
        self->scratch_cap = self->cache_cap;
        self->scratch = (uint8_t*)REALLOCATE(
                            self->scratch, self->scratch_cap * width);
    }

    // Exploit previous sorting, rather than sort cache naively.
    // Leave the first slice intact if the number of slices is odd. */
    while (self->num_slices > 1) {
        uint32_t i = 0;
        uint32_t j = 0;

        while (i < self->num_slices) {
            if (self->num_slices - i >= 2) {
                // Merge two consecutive slices.
                const uint32_t merged_size = slice_sizes[i] + slice_sizes[i + 1];
                Sort_merge(slice_starts[i], slice_sizes[i],
                           slice_starts[i + 1], slice_sizes[i + 1], self->scratch,
                           self->width, compare, self);
                slice_sizes[j]  = merged_size;
                slice_starts[j] = slice_starts[i];
                memcpy(slice_starts[j], self->scratch, merged_size * width);
                i += 2;
                j += 1;
            }
            else if (self->num_slices - i >= 1) {
                // Move single slice pointer.
                slice_sizes[j]  = slice_sizes[i];
                slice_starts[j] = slice_starts[i];
                i += 1;
                j += 1;
            }
        }
        self->num_slices = j;
    }

    self->num_slices = 0;
}

void
SortEx_grow_cache(SortExternal *self, uint32_t size) {
    if (size > self->cache_cap) {
        self->cache = (uint8_t*)REALLOCATE(self->cache, size * self->width);
        self->cache_cap = size;
    }
}

static uint32_t
S_find_slice_size(SortExternal *self, uint8_t *endpost) {
    int32_t          lo      = self->cache_tick - 1;
    int32_t          hi      = self->cache_max;
    uint8_t *const   cache   = self->cache;
    const size_t     width   = self->width;
    SortEx_compare_t compare
        = (SortEx_compare_t)METHOD(SortEx_Get_VTable(self), SortEx, Compare);

    // Binary search.
    while (hi - lo > 1) {
        const int32_t mid   = lo + ((hi - lo) / 2);
        const int32_t delta = compare(self, cache + mid * width, endpost);
        if (delta > 0) { hi = mid; }
        else           { lo = mid; }
    }

    // If lo is still -1, we didn't find anything.
    return lo == -1
           ? 0
           : (lo - self->cache_tick) + 1;
}

void
SortEx_set_mem_thresh(SortExternal *self, uint32_t mem_thresh) {
    self->mem_thresh = mem_thresh;
}

uint32_t
SortEx_cache_count(SortExternal *self) {
    return self->cache_max - self->cache_tick;
}


