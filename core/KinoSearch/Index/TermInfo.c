#define C_KINO_TERMINFO
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/TermInfo.h"
#include "KinoSearch/Util/StringHelper.h"

TermInfo*
TInfo_new(int32_t doc_freq)
{
    TermInfo *self = (TermInfo*)VTable_Make_Obj(TERMINFO);
    return TInfo_init(self, doc_freq);
}

TermInfo*
TInfo_init(TermInfo *self, int32_t doc_freq)
{
    self->doc_freq      = doc_freq;
    self->post_filepos  = 0;
    self->skip_filepos  = 0;
    self->lex_filepos   = 0;
    return self;
}

TermInfo*
TInfo_clone(TermInfo *self) 
{
    TermInfo *evil_twin = TInfo_new(self->doc_freq);
    evil_twin->post_filepos = self->post_filepos;
    evil_twin->skip_filepos = self->skip_filepos;
    evil_twin->lex_filepos  = self->lex_filepos;
    return evil_twin;
}

int32_t
TInfo_get_doc_freq(TermInfo *self)     { return self->doc_freq; }
int64_t
TInfo_get_lex_filepos(TermInfo *self)  { return self->lex_filepos; }
int64_t
TInfo_get_post_filepos(TermInfo *self) { return self->post_filepos; }
int64_t
TInfo_get_skip_filepos(TermInfo *self) { return self->skip_filepos; }

void
TInfo_set_doc_freq(TermInfo *self, int32_t doc_freq)
    { self->doc_freq = doc_freq; }
void
TInfo_set_lex_filepos(TermInfo *self, int64_t filepos)
    { self->lex_filepos = filepos; }
void
TInfo_set_post_filepos(TermInfo *self, int64_t filepos)
    { self->post_filepos = filepos; }
void
TInfo_set_skip_filepos(TermInfo *self, int64_t filepos)
    { self->skip_filepos = filepos; }

// TODO: this should probably be some sort of Dump variant rather than
// To_String.
CharBuf*
TInfo_to_string(TermInfo *self)
{
    return CB_newf(
        "doc freq:      %i32\n"
        "post filepos:  %i64\n"
        "skip filepos:  %i64\n" 
        "index filepos: %i64",
        self->doc_freq, self->post_filepos,
        self->skip_filepos, self->lex_filepos
    );
}

void
TInfo_mimic(TermInfo *self, Obj *other) 
{
    TermInfo *evil_twin = (TermInfo*)CERTIFY(other, TERMINFO);
    self->doc_freq      = evil_twin->doc_freq;
    self->post_filepos  = evil_twin->post_filepos;
    self->skip_filepos  = evil_twin->skip_filepos;
    self->lex_filepos   = evil_twin->lex_filepos;
}

void
TInfo_reset(TermInfo *self) 
{
    self->doc_freq      = 0;
    self->post_filepos  = 0;
    self->skip_filepos  = 0;
    self->lex_filepos   = 0;
}


