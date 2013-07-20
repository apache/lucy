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
S_refill_cache(SortExternal *self, SortExternalIVARS *ivars);

// Absorb all the items which are "in-range" from all the Runs into the main
// cache.
static void
S_absorb_slices(SortExternal *self, SortExternalIVARS *ivars,
                uint8_t *endpost);

// Return the address for the item in one of the runs' caches which is the
// highest in sort order, but which we can guarantee is lower in sort order
// than any item which has yet to enter a run cache.
static uint8_t*
S_find_endpost(SortExternal *self, SortExternalIVARS *ivars);

// Determine how many cache items are less than or equal to [endpost].
static uint32_t
S_find_slice_size(SortExternal *self, SortExternalIVARS *ivars,
                  uint8_t *endpost);

SortExternal*
SortEx_init(SortExternal *self, size_t width) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    // Assign.
    ivars->width        = width;

    // Init.
    ivars->mem_thresh   = UINT32_MAX;
    ivars->cache        = NULL;
    ivars->cache_cap    = 0;
    ivars->cache_max    = 0;
    ivars->cache_tick   = 0;
    ivars->scratch      = NULL;
    ivars->scratch_cap  = 0;
    ivars->runs         = VA_new(0);
    ivars->slice_sizes  = NULL;
    ivars->slice_starts = NULL;
    ivars->num_slices   = 0;
    ivars->flipped      = false;

    ABSTRACT_CLASS_CHECK(self, SORTEXTERNAL);
    return self;
}

void
SortEx_destroy(SortExternal *self) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    FREEMEM(ivars->scratch);
    FREEMEM(ivars->slice_sizes);
    FREEMEM(ivars->slice_starts);
    if (ivars->cache) {
        SortEx_Clear_Cache(self);
        FREEMEM(ivars->cache);
    }
    DECREF(ivars->runs);
    SUPER_DESTROY(self, SORTEXTERNAL);
}

void
SortEx_clear_cache(SortExternal *self) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    ivars->cache_max    = 0;
    ivars->cache_tick   = 0;
}

void
SortEx_feed(SortExternal *self, void *data) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    const size_t width = ivars->width;
    if (ivars->cache_max == ivars->cache_cap) {
        size_t amount = Memory_oversize(ivars->cache_max + 1, width);
        SortEx_Grow_Cache(self, amount);
    }
    uint8_t *target = ivars->cache + ivars->cache_max * width;
    memcpy(target, data, width);
    ivars->cache_max++;
}

static CFISH_INLINE void*
SI_peek(SortExternal *self, SortExternalIVARS *ivars) {
    if (ivars->cache_tick >= ivars->cache_max) {
        S_refill_cache(self, ivars);
    }

    if (ivars->cache_max > 0) {
        return ivars->cache + ivars->cache_tick * ivars->width;
    }
    else {
        return NULL;
    }
}

void*
SortEx_fetch(SortExternal *self) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    void *address = SI_peek(self, ivars);
    ivars->cache_tick++;
    return address;
}

void*
SortEx_peek(SortExternal *self) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    return SI_peek(self, ivars);
}

void
SortEx_sort_cache(SortExternal *self) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    if (ivars->cache_tick != 0) {
        THROW(ERR, "Cant Sort_Cache() after fetching %u32 items", ivars->cache_tick);
    }
    if (ivars->cache_max != 0) {
        VTable *vtable = SortEx_Get_VTable(self);
        Cfish_Sort_Compare_t compare
            = (Cfish_Sort_Compare_t)METHOD_PTR(vtable, Lucy_SortEx_Compare);
        if (ivars->scratch_cap < ivars->cache_cap) {
            ivars->scratch_cap = ivars->cache_cap;
            ivars->scratch
                = (uint8_t*)REALLOCATE(ivars->scratch,
                                       ivars->scratch_cap * ivars->width);
        }
        Sort_mergesort(ivars->cache, ivars->scratch, ivars->cache_max,
                       ivars->width, compare, self);
    }
}

void
SortEx_flip(SortExternal *self) {
    SortEx_Flush(self);
    SortEx_IVARS(self)->flipped = true;
}

void
SortEx_add_run(SortExternal *self, SortExternal *run) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    VA_Push(ivars->runs, (Obj*)run);
    uint32_t num_runs = VA_Get_Size(ivars->runs);
    ivars->slice_sizes
        = (uint32_t*)REALLOCATE(ivars->slice_sizes,
                                num_runs * sizeof(uint32_t));
    ivars->slice_starts
        = (uint8_t**)REALLOCATE(ivars->slice_starts,
                                num_runs * sizeof(uint8_t*));
}

static void
S_refill_cache(SortExternal *self, SortExternalIVARS *ivars) {
    // Reset cache vars.
    SortEx_Clear_Cache(self);

    // Make sure all runs have at least one item in the cache.
    uint32_t i = 0;
    while (i < VA_Get_Size(ivars->runs)) {
        SortExternal *const run = (SortExternal*)VA_Fetch(ivars->runs, i);
        if (SortEx_Cache_Count(run) > 0 || SortEx_Refill(run) > 0) {
            i++; // Run has some elements, so keep.
        }
        else {
            VA_Excise(ivars->runs, i, 1);
        }
    }

    // Absorb as many elems as possible from all runs into main cache.
    if (VA_Get_Size(ivars->runs)) {
        uint8_t *endpost = S_find_endpost(self, ivars);
        S_absorb_slices(self, ivars, endpost);
    }
}

static uint8_t*
S_find_endpost(SortExternal *self, SortExternalIVARS *ivars) {
    uint8_t *endpost = NULL;
    const size_t width = ivars->width;

    for (uint32_t i = 0, max = VA_Get_Size(ivars->runs); i < max; i++) {
        // Get a run and retrieve the last item in its cache.
        SortExternal *const run = (SortExternal*)VA_Fetch(ivars->runs, i);
        SortExternalIVARS *const run_ivars = SortEx_IVARS(run);
        const uint32_t tick = run_ivars->cache_max - 1;
        if (tick >= run_ivars->cache_cap || run_ivars->cache_max < 1) {
            THROW(ERR, "Invalid SortExternal cache access: %u32 %u32 %u32", tick,
                  run_ivars->cache_max, run_ivars->cache_cap);
        }
        else {
            // Cache item with the highest sort value currently held in memory
            // by the run.
            uint8_t *candidate = run_ivars->cache + tick * width;

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
S_absorb_slices(SortExternal *self, SortExternalIVARS *ivars,
                uint8_t *endpost) {
    size_t      width        = ivars->width;
    uint32_t    num_runs     = VA_Get_Size(ivars->runs);
    uint8_t   **slice_starts = ivars->slice_starts;
    uint32_t   *slice_sizes  = ivars->slice_sizes;
    VTable     *vtable       = SortEx_Get_VTable(self);
    Cfish_Sort_Compare_t compare
        = (Cfish_Sort_Compare_t)METHOD_PTR(vtable, Lucy_SortEx_Compare);

    if (ivars->cache_max != 0) { THROW(ERR, "Can't refill unless empty"); }

    // Move all the elements in range into the main cache as slices.
    for (uint32_t i = 0; i < num_runs; i++) {
        SortExternal *const run = (SortExternal*)VA_Fetch(ivars->runs, i);
        SortExternalIVARS *const run_ivars = SortEx_IVARS(run);
        uint32_t slice_size = S_find_slice_size(run, run_ivars, endpost);

        if (slice_size) {
            // Move slice content from run cache to main cache.
            if (ivars->cache_max + slice_size > ivars->cache_cap) {
                size_t cap = Memory_oversize(ivars->cache_max + slice_size,
                                             width);
                SortEx_Grow_Cache(self, cap);
            }
            memcpy(ivars->cache + ivars->cache_max * width,
                   run_ivars->cache + run_ivars->cache_tick * width,
                   slice_size * width);
            run_ivars->cache_tick += slice_size;
            ivars->cache_max += slice_size;

            // Track number of slices and slice sizes.
            slice_sizes[ivars->num_slices++] = slice_size;
        }
    }

    // Transform slice starts from ticks to pointers.
    uint32_t total = 0;
    for (uint32_t i = 0; i < ivars->num_slices; i++) {
        slice_starts[i] = ivars->cache + total * width;
        total += slice_sizes[i];
    }

    // The main cache now consists of several slices.  Sort the main cache,
    // but exploit the fact that each slice is already sorted.
    if (ivars->scratch_cap < ivars->cache_cap) {
        ivars->scratch_cap = ivars->cache_cap;
        ivars->scratch = (uint8_t*)REALLOCATE(
                            ivars->scratch, ivars->scratch_cap * width);
    }

    // Exploit previous sorting, rather than sort cache naively.
    // Leave the first slice intact if the number of slices is odd. */
    while (ivars->num_slices > 1) {
        uint32_t i = 0;
        uint32_t j = 0;

        while (i < ivars->num_slices) {
            if (ivars->num_slices - i >= 2) {
                // Merge two consecutive slices.
                const uint32_t merged_size = slice_sizes[i] + slice_sizes[i + 1];
                Sort_merge(slice_starts[i], slice_sizes[i],
                           slice_starts[i + 1], slice_sizes[i + 1], ivars->scratch,
                           ivars->width, compare, self);
                slice_sizes[j]  = merged_size;
                slice_starts[j] = slice_starts[i];
                memcpy(slice_starts[j], ivars->scratch, merged_size * width);
                i += 2;
                j += 1;
            }
            else if (ivars->num_slices - i >= 1) {
                // Move single slice pointer.
                slice_sizes[j]  = slice_sizes[i];
                slice_starts[j] = slice_starts[i];
                i += 1;
                j += 1;
            }
        }
        ivars->num_slices = j;
    }

    ivars->num_slices = 0;
}

void
SortEx_grow_cache(SortExternal *self, uint32_t size) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    if (size > ivars->cache_cap) {
        ivars->cache = (uint8_t*)REALLOCATE(ivars->cache, size * ivars->width);
        ivars->cache_cap = size;
    }
}

static uint32_t
S_find_slice_size(SortExternal *self, SortExternalIVARS *ivars,
                  uint8_t *endpost) {
    int32_t          lo      = ivars->cache_tick - 1;
    int32_t          hi      = ivars->cache_max;
    uint8_t *const   cache   = ivars->cache;
    const size_t     width   = ivars->width;
    SortEx_Compare_t compare
        = METHOD_PTR(SortEx_Get_VTable(self), Lucy_SortEx_Compare);

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
           : (lo - ivars->cache_tick) + 1;
}

void
SortEx_set_mem_thresh(SortExternal *self, uint32_t mem_thresh) {
    SortEx_IVARS(self)->mem_thresh = mem_thresh;
}

uint32_t
SortEx_cache_count(SortExternal *self) {
    SortExternalIVARS *const ivars = SortEx_IVARS(self);
    return ivars->cache_max - ivars->cache_tick;
}


