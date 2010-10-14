#define C_KINO_SERIESMATCHER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/SeriesMatcher.h"

SeriesMatcher*
SeriesMatcher_new(VArray *matchers, I32Array *offsets)
{
    SeriesMatcher *self = (SeriesMatcher*)VTable_Make_Obj(SERIESMATCHER);
    return SeriesMatcher_init(self, matchers, offsets);
}

SeriesMatcher*
SeriesMatcher_init(SeriesMatcher *self, VArray *matchers, I32Array *offsets)
{
    Matcher_init((Matcher*)self);

    // Init. 
    self->current_matcher = NULL;
    self->current_offset  = 0;
    self->next_offset     = 0;
    self->doc_id          = 0;
    self->tick            = 0;

    // Assign. 
    self->matchers        = (VArray*)INCREF(matchers);
    self->offsets         = (I32Array*)INCREF(offsets);

    // Derive. 
    self->num_matchers    = (int32_t)I32Arr_Get_Size(offsets);

    return self;
}

void
SeriesMatcher_destroy(SeriesMatcher *self)
{
    DECREF(self->matchers);
    DECREF(self->offsets);
    SUPER_DESTROY(self, SERIESMATCHER);
}

int32_t
SeriesMatcher_next(SeriesMatcher *self)
{
    return SeriesMatcher_advance(self, self->doc_id + 1);
}

int32_t
SeriesMatcher_advance(SeriesMatcher *self, int32_t target) 
{
    if (target >= self->next_offset) {
        // Proceed to next matcher or bail. 
        if (self->tick < self->num_matchers) {
            while (1) {
                uint32_t next_offset = self->tick + 1 == self->num_matchers
                    ? I32_MAX 
                    : I32Arr_Get(self->offsets, self->tick + 1);
                self->current_matcher = (Matcher*)VA_Fetch(self->matchers,
                    self->tick);
                self->current_offset = self->next_offset;
                self->next_offset = next_offset;
                self->doc_id = next_offset - 1;
                self->tick++;
                if (   self->current_matcher != NULL 
                    || self->tick >= self->num_matchers
                ) {
                    break;
                }
            } 
            return SeriesMatcher_advance(self, target); // Recurse. 
        }
        else {
            // We're done. 
            self->doc_id = 0;
            return 0;
        }
    }
    else {
        int32_t target_minus_offset = target - self->current_offset;
        int32_t found 
            = Matcher_Advance(self->current_matcher, target_minus_offset);
        if (found) {
            self->doc_id = found + self->current_offset;
            return self->doc_id;
        }
        else {
            // Recurse. 
            return SeriesMatcher_advance(self, self->next_offset);
        }
    }
}

int32_t 
SeriesMatcher_get_doc_id(SeriesMatcher *self) { return self->doc_id; }

/* Copyright 2007-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

