#define C_KINO_DOCVECTOR
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/DocVector.h"
#include "KinoSearch/Index/TermVector.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"

// Extract a document's compressed TermVector data into ( term_text =>
// compressed positional data ) pairs.
static Hash*
S_extract_tv_cache(ByteBuf *field_buf);

// Pull a TermVector object out from compressed positional data.
static TermVector*
S_extract_tv_from_tv_buf(const CharBuf *field, const CharBuf *term_text, 
                         ByteBuf *tv_buf);

DocVector*
DocVec_new()
{
    DocVector *self = (DocVector*)VTable_Make_Obj(DOCVECTOR);
    return DocVec_init(self);
}

DocVector*
DocVec_init(DocVector *self)
{
    self->field_bufs    = Hash_new(0);
    self->field_vectors = Hash_new(0);
    return self;
}

void
DocVec_serialize(DocVector *self, OutStream *outstream)
{
    Hash_Serialize(self->field_bufs, outstream);
    Hash_Serialize(self->field_vectors, outstream);
}

DocVector*
DocVec_deserialize(DocVector *self, InStream *instream)
{
    self = self ? self : (DocVector*)VTable_Make_Obj(DOCVECTOR);
    self->field_bufs    = Hash_deserialize(NULL, instream);
    self->field_vectors = Hash_deserialize(NULL, instream);
    return self;
}

void
DocVec_destroy(DocVector *self)
{
    DECREF(self->field_bufs);
    DECREF(self->field_vectors);
    SUPER_DESTROY(self, DOCVECTOR);
}

void
DocVec_add_field_buf(DocVector *self, const CharBuf *field, 
                     ByteBuf *field_buf)
{
    Hash_Store(self->field_bufs, (Obj*)field, INCREF(field_buf));
}

ByteBuf*
DocVec_field_buf(DocVector *self, const CharBuf *field)
{
    return (ByteBuf*)Hash_Fetch(self->field_bufs, (Obj*)field);
}

VArray*
DocVec_field_names(DocVector *self)
{
    return Hash_Keys(self->field_bufs);
}

TermVector*
DocVec_term_vector(DocVector *self, const CharBuf *field, 
                   const CharBuf *term_text) 
{
    ByteBuf *tv_buf;
    Hash *field_vector = (Hash*)Hash_Fetch(self->field_vectors, (Obj*)field);
    
    // If no cache hit, try to fill cache. 
    if (field_vector == NULL) {
        ByteBuf *field_buf 
            = (ByteBuf*)Hash_Fetch(self->field_bufs, (Obj*)field);

        // Bail if there's no content or the field isn't highlightable. 
        if (field_buf == NULL) return NULL;

        field_vector = S_extract_tv_cache(field_buf);
        Hash_Store(self->field_vectors, (Obj*)field, (Obj*)field_vector);
    }

    // Get a buf for the term text or bail. 
    tv_buf = (ByteBuf*)Hash_Fetch(field_vector, (Obj*)term_text);
    if (tv_buf == NULL) 
        return NULL;

    return S_extract_tv_from_tv_buf(field, term_text, tv_buf);
}

static Hash*
S_extract_tv_cache(ByteBuf *field_buf) 
{
    Hash          *tv_cache  = Hash_new(0);
    char          *tv_string = BB_Get_Buf(field_buf);
    int32_t        num_terms = NumUtil_decode_c32(&tv_string);
    CharBuf       *text      = CB_new(0);
    int32_t        i;
    
    // Read the number of highlightable terms in the field. 
    for (i = 0; i < num_terms; i++) {
        char         *bookmark_ptr;
        size_t        overlap = NumUtil_decode_c32(&tv_string);
        size_t        len     = NumUtil_decode_c32(&tv_string);
        int32_t       num_positions;

        // Decompress the term text. 
        CB_Set_Size(text, overlap);
        CB_Cat_Trusted_Str(text, tv_string, len);
        tv_string += len;

        // Get positions & offsets string. 
        bookmark_ptr  = tv_string;
        num_positions = NumUtil_decode_c32(&tv_string);
        while(num_positions--) {
            // Leave nums compressed to save a little mem. 
            NumUtil_skip_cint(&tv_string);
            NumUtil_skip_cint(&tv_string);
            NumUtil_skip_cint(&tv_string);
        }
        len = tv_string - bookmark_ptr;

        // Store the $text => $posdata pair in the output hash. 
        Hash_Store(tv_cache, (Obj*)text,
            (Obj*)BB_new_bytes(bookmark_ptr, len));
    }
    DECREF(text);

    return tv_cache;
}

static TermVector*
S_extract_tv_from_tv_buf(const CharBuf *field, const CharBuf *term_text, 
                         ByteBuf *tv_buf)
{
    TermVector *retval      = NULL;
    char       *posdata     = BB_Get_Buf(tv_buf);
    char       *posdata_end = posdata + BB_Get_Size(tv_buf);
    int32_t    *positions   = NULL;
    int32_t    *starts      = NULL;
    int32_t    *ends        = NULL;
    uint32_t    num_pos     = 0;
    uint32_t    i;

    if (posdata != posdata_end) {
        num_pos   = NumUtil_decode_c32(&posdata);
        positions = (int32_t*)MALLOCATE(num_pos * sizeof(int32_t));
        starts    = (int32_t*)MALLOCATE(num_pos * sizeof(int32_t));
        ends      = (int32_t*)MALLOCATE(num_pos * sizeof(int32_t));
    }

    // Expand C32s. 
    for (i = 0; i < num_pos; i++) {
        positions[i] = NumUtil_decode_c32(&posdata);
        starts[i]    = NumUtil_decode_c32(&posdata);
        ends[i]      = NumUtil_decode_c32(&posdata);
    }

    if (posdata != posdata_end) {
        THROW(ERR, "Bad encoding of posdata");
    }
    else {
        I32Array *posits_map = I32Arr_new_steal(positions, num_pos);
        I32Array *starts_map = I32Arr_new_steal(starts, num_pos);
        I32Array *ends_map   = I32Arr_new_steal(ends, num_pos);
        retval = TV_new(field, term_text, posits_map, starts_map, ends_map);
        DECREF(posits_map);
        DECREF(starts_map);
        DECREF(ends_map);
    }

    return retval;
}


