#define C_KINO_STEMMER
#define C_KINO_TOKEN
#include <ctype.h>
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Analysis/Stemmer.h"
#include "KinoSearch/Analysis/Token.h"
#include "KinoSearch/Analysis/Inversion.h"

Stemmer_sb_stemmer_new_t    Stemmer_sb_stemmer_new    = NULL;
Stemmer_sb_stemmer_delete_t Stemmer_sb_stemmer_delete = NULL;
Stemmer_sb_stemmer_stem_t   Stemmer_sb_stemmer_stem   = NULL;
Stemmer_sb_stemmer_length_t Stemmer_sb_stemmer_length = NULL;

Stemmer*
Stemmer_new(const CharBuf *language)
{
    Stemmer *self = (Stemmer*)VTable_Make_Obj(STEMMER);
    return Stemmer_init(self, language);
}

Stemmer*
Stemmer_init(Stemmer *self, const CharBuf *language)
{
    char lang_buf[3];
    Analyzer_init((Analyzer*)self);
    self->language = CB_Clone(language);

    // Get a Snowball stemmer.  Be case-insensitive. 
    Stemmer_load_snowball();
    lang_buf[0] = tolower(CB_Code_Point_At(language, 0));
    lang_buf[1] = tolower(CB_Code_Point_At(language, 1));
    lang_buf[2] = '\0';
    self->snowstemmer = kino_Stemmer_sb_stemmer_new(lang_buf, "UTF_8");
    if (!self->snowstemmer) 
        THROW(ERR, "Can't find a Snowball stemmer for %o", language);

    return self;
}

void
Stemmer_destroy(Stemmer *self)
{
    if (self->snowstemmer) {
        kino_Stemmer_sb_stemmer_delete((struct sb_stemmer*)self->snowstemmer);
    }
    DECREF(self->language);
    SUPER_DESTROY(self, STEMMER);
}

Inversion*
Stemmer_transform(Stemmer *self, Inversion *inversion)
{
    Token *token;
    struct sb_stemmer *const snowstemmer 
        = (struct sb_stemmer*)self->snowstemmer;

    while (NULL != (token = Inversion_Next(inversion))) {
        sb_symbol *stemmed_text = kino_Stemmer_sb_stemmer_stem(snowstemmer, 
            (sb_symbol*)token->text, token->len);
        size_t len = kino_Stemmer_sb_stemmer_length(snowstemmer);
        if (len > token->len) {
            FREEMEM(token->text);
            token->text = (char*)MALLOCATE(len + 1);
        }
        memcpy(token->text, stemmed_text, len + 1);
        token->len = len;
    }
    Inversion_Reset(inversion);
    return (Inversion*)INCREF(inversion);
}

Hash*
Stemmer_dump(Stemmer *self)
{
    Stemmer_dump_t super_dump 
        = (Stemmer_dump_t)SUPER_METHOD(STEMMER, Stemmer, Dump);
    Hash *dump = super_dump(self);
    Hash_Store_Str(dump, "language", 8, (Obj*)CB_Clone(self->language));
    return dump;
}

Stemmer*
Stemmer_load(Stemmer *self, Obj *dump)
{
    Stemmer_load_t super_load 
        = (Stemmer_load_t)SUPER_METHOD(STEMMER, Stemmer, Load);
    Stemmer *loaded = super_load(self, dump);
    Hash    *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *language = (CharBuf*)CERTIFY(
        Hash_Fetch_Str(source, "language", 8), CHARBUF);
    return Stemmer_init(loaded, language);
}

bool_t
Stemmer_equals(Stemmer *self, Obj *other)
{
    Stemmer *const evil_twin = (Stemmer*)other;
    if (evil_twin == self) return true;
    if (!Obj_Is_A(other, STEMMER)) return false;
    if (!CB_Equals(evil_twin->language, (Obj*)self->language)) return false;
    return true;
} 


