#define C_KINO_LEXICON
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/Lexicon.h"

Lexicon*
Lex_init(Lexicon *self, const CharBuf *field)
{
    self->field = CB_Clone(field);
    ABSTRACT_CLASS_CHECK(self, LEXICON);
    return self;
}

CharBuf*
Lex_get_field(Lexicon *self) { return self->field; }

void
Lex_destroy(Lexicon *self)
{
    DECREF(self->field);
    SUPER_DESTROY(self, LEXICON);
}


