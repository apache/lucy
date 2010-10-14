#define C_KINO_MATCHER
#define KINO_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "KinoSearch/Search/Matcher.h"
#include "KinoSearch/Object/Err.h"
#include "KinoSearch/Object/VTable.h"
#include "KinoSearch/Search/Collector.h"

Matcher*
Matcher_init(Matcher *self)
{
    ABSTRACT_CLASS_CHECK(self, MATCHER);
    return self;
}

int32_t
Matcher_advance(Matcher *self, int32_t target) 
{
    while (1) {
        int32_t doc_id = Matcher_Next(self);
        if (doc_id == 0 || doc_id >= target)
            return doc_id; 
    }
}

void
Matcher_collect(Matcher *self, Collector *collector, Matcher *deletions)
{
    int32_t doc_id         = 0;
    int32_t next_deletion  = deletions ? 0 : I32_MAX;

    Coll_Set_Matcher(collector, self);

    // Execute scoring loop. 
    while (1) {
        if (doc_id > next_deletion) {
            next_deletion = Matcher_Advance(deletions, doc_id);
            if (next_deletion == 0) { next_deletion = I32_MAX; }
            continue;
        }
        else if (doc_id == next_deletion) {
            // Skip past deletions. 
            while (doc_id == next_deletion) {
                // Artifically advance matcher. 
                while (doc_id == next_deletion) {
                    doc_id++;
                    next_deletion = Matcher_Advance(deletions, doc_id);
                    if (next_deletion == 0) { next_deletion = I32_MAX; }
                }
                // Verify that the artificial advance actually worked. 
                doc_id = Matcher_Advance(self, doc_id);
                if (doc_id > next_deletion) {
                    next_deletion = Matcher_Advance(deletions, doc_id);
                }
            }
        }
        else {
            doc_id = Matcher_Advance(self, doc_id + 1);
            if (doc_id >= next_deletion) { 
                next_deletion = Matcher_Advance(deletions, doc_id);
                if (doc_id == next_deletion) { continue; }
            }
        }

        if (doc_id) {
            Coll_Collect(collector, doc_id);
        }
        else { 
            break; 
        }
    }

    Coll_Set_Matcher(collector, NULL);
}

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

