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

#define C_LUCY_HIGHLIGHTWRITER
#define C_LUCY_DEFAULTHIGHLIGHTWRITER
#define C_LUCY_TOKEN
#include "Lucy/Util/ToolSet.h"

#include <stdio.h>

#include "Lucy/Index/HighlightWriter.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Index/HighlightReader.h"
#include "Lucy/Index/Inverter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/InStream.h"

static OutStream*
S_lazy_init(HighlightWriter *self);

int32_t HLWriter_current_file_format = 1;

HighlightWriter*
HLWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
             PolyReader *polyreader) {
    HighlightWriter *self
        = (HighlightWriter*)VTable_Make_Obj(HIGHLIGHTWRITER);
    return HLWriter_init(self, schema, snapshot, segment, polyreader);
}

HighlightWriter*
HLWriter_init(HighlightWriter *self, Schema *schema, Snapshot *snapshot,
              Segment *segment, PolyReader *polyreader) {
    DataWriter_init((DataWriter*)self, schema, snapshot, segment, polyreader);
    return self;
}

void
HLWriter_destroy(HighlightWriter *self) {
    DECREF(self->dat_out);
    DECREF(self->ix_out);
    SUPER_DESTROY(self, HIGHLIGHTWRITER);
}

static OutStream*
S_lazy_init(HighlightWriter *self) {
    if (!self->dat_out) {
        Segment  *segment  = self->segment;
        Folder   *folder   = self->folder;
        CharBuf  *seg_name = Seg_Get_Name(segment);

        // Open outstreams.
        {
            CharBuf *ix_file = CB_newf("%o/highlight.ix", seg_name);
            self->ix_out = Folder_Open_Out(folder, ix_file);
            DECREF(ix_file);
            if (!self->ix_out) { RETHROW(INCREF(Err_get_error())); }
        }
        {
            CharBuf *dat_file = CB_newf("%o/highlight.dat", seg_name);
            self->dat_out = Folder_Open_Out(folder, dat_file);
            DECREF(dat_file);
            if (!self->dat_out) { RETHROW(INCREF(Err_get_error())); }
        }

        // Go past invalid doc 0.
        OutStream_Write_I64(self->ix_out, 0);
    }

    return self->dat_out;
}

void
HLWriter_add_inverted_doc(HighlightWriter *self, Inverter *inverter,
                          int32_t doc_id) {
    OutStream *dat_out = S_lazy_init(self);
    OutStream *ix_out  = self->ix_out;
    int64_t    filepos = OutStream_Tell(dat_out);
    uint32_t num_highlightable = 0;
    int32_t expected = (int32_t)(OutStream_Tell(ix_out) / 8);

    // Verify doc id.
    if (doc_id != expected) {
        THROW(ERR, "Expected doc id %i32 but got %i32", expected, doc_id);
    }

    // Write index data.
    OutStream_Write_I64(ix_out, filepos);

    // Count, then write number of highlightable fields.
    Inverter_Iterate(inverter);
    while (Inverter_Next(inverter)) {
        FieldType *type = Inverter_Get_Type(inverter);
        if (FType_Is_A(type, FULLTEXTTYPE)
            && FullTextType_Highlightable((FullTextType*)type)
           ) {
            num_highlightable++;
        }
    }
    OutStream_Write_C32(dat_out, num_highlightable);

    Inverter_Iterate(inverter);
    while (Inverter_Next(inverter)) {
        FieldType *type = Inverter_Get_Type(inverter);
        if (FType_Is_A(type, FULLTEXTTYPE)
            && FullTextType_Highlightable((FullTextType*)type)
           ) {
            CharBuf   *field     = Inverter_Get_Field_Name(inverter);
            Inversion *inversion = Inverter_Get_Inversion(inverter);
            ByteBuf   *tv_buf    = HLWriter_TV_Buf(self, inversion);
            CB_Serialize(field, dat_out);
            BB_Serialize(tv_buf, dat_out);
            DECREF(tv_buf);
        }
    }
}

ByteBuf*
HLWriter_tv_buf(HighlightWriter *self, Inversion *inversion) {
    char       *last_text = "";
    size_t      last_len = 0;
    ByteBuf    *tv_buf = BB_new(20 + Inversion_Get_Size(inversion) * 8);
    uint32_t    num_postings = 0;
    char       *dest;
    Token     **tokens;
    uint32_t    freq;
    UNUSED_VAR(self);

    // Leave space for a c32 indicating the number of postings.
    BB_Set_Size(tv_buf, C32_MAX_BYTES);

    Inversion_Reset(inversion);
    while ((tokens = Inversion_Next_Cluster(inversion, &freq)) != NULL) {
        Token *token = *tokens;
        int32_t overlap = StrHelp_overlap(last_text, token->text,
                                          last_len, token->len);
        char *ptr;
        char *orig;
        size_t old_size = BB_Get_Size(tv_buf);
        size_t new_size = old_size
                          + C32_MAX_BYTES      // overlap
                          + C32_MAX_BYTES      // length of string diff
                          + (token->len - overlap) // diff char data
                          + C32_MAX_BYTES                // num prox
                          + (C32_MAX_BYTES * freq * 3);  // pos data

        // Allocate for worst-case scenario.
        ptr  = BB_Grow(tv_buf, new_size);
        orig = ptr;
        ptr += old_size;

        // Track number of postings.
        num_postings += 1;

        // Append the string diff to the tv_buf.
        NumUtil_encode_c32(overlap, &ptr);
        NumUtil_encode_c32((token->len - overlap), &ptr);
        memcpy(ptr, (token->text + overlap), (token->len - overlap));
        ptr += token->len - overlap;

        // Save text and text_len for comparison next loop.
        last_text = token->text;
        last_len  = token->len;

        // Append the number of positions for this term.
        NumUtil_encode_c32(freq, &ptr);

        do {
            // Add position, start_offset, and end_offset to tv_buf.
            NumUtil_encode_c32(token->pos, &ptr);
            NumUtil_encode_c32(token->start_offset, &ptr);
            NumUtil_encode_c32(token->end_offset, &ptr);

        } while (--freq && (token = *++tokens));

        // Set new byte length.
        BB_Set_Size(tv_buf, ptr - orig);
    }

    // Go back and start the term vector string with the posting count.
    dest = BB_Get_Buf(tv_buf);
    NumUtil_encode_padded_c32(num_postings, &dest);

    return tv_buf;
}

void
HLWriter_add_segment(HighlightWriter *self, SegReader *reader,
                     I32Array *doc_map) {
    int32_t doc_max = SegReader_Doc_Max(reader);

    if (doc_max == 0) {
        // Bail if the supplied segment is empty.
        return;
    }
    else {
        DefaultHighlightReader *hl_reader
            = (DefaultHighlightReader*)CERTIFY(
                  SegReader_Obtain(reader, VTable_Get_Name(HIGHLIGHTREADER)),
                  DEFAULTHIGHLIGHTREADER);
        OutStream *dat_out = S_lazy_init(self);
        OutStream *ix_out  = self->ix_out;
        int32_t    orig;
        ByteBuf   *bb = BB_new(0);

        for (orig = 1; orig <= doc_max; orig++) {
            // Skip deleted docs.
            if (doc_map && !I32Arr_Get(doc_map, orig)) {
                continue;
            }

            // Write file pointer.
            OutStream_Write_I64(ix_out, OutStream_Tell(dat_out));

            // Copy the raw record.
            DefHLReader_Read_Record(hl_reader, orig, bb);
            OutStream_Write_Bytes(dat_out, BB_Get_Buf(bb), BB_Get_Size(bb));

            BB_Set_Size(bb, 0);
        }
        DECREF(bb);
    }
}

void
HLWriter_finish(HighlightWriter *self) {
    if (self->dat_out) {
        // Write one final file pointer, so that we can derive the length of
        // the last record.
        int64_t end = OutStream_Tell(self->dat_out);
        OutStream_Write_I64(self->ix_out, end);

        // Close down the output streams.
        OutStream_Close(self->dat_out);
        OutStream_Close(self->ix_out);
        Seg_Store_Metadata_Str(self->segment, "highlight", 9,
                               (Obj*)HLWriter_Metadata(self));
    }
}

int32_t
HLWriter_format(HighlightWriter *self) {
    UNUSED_VAR(self);
    return HLWriter_current_file_format;
}


