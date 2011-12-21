# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

package Lucy::Analysis::Token;
use Lucy;

1;

__END__

__BINDING__

my $xs = <<'END_XS';
MODULE = Lucy    PACKAGE = Lucy::Analysis::Token

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    SV       *text_sv   = NULL;
    uint32_t  start_off = 0;
    uint32_t  end_off   = 0;
    int32_t   pos_inc   = 1;
    float     boost     = 1.0f;

    chy_bool_t args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              "Lucy::Analysis::Token::new_PARAMS",
                              ALLOT_SV(&text_sv, "text", 4, true),
                              ALLOT_U32(&start_off, "start_offset", 12, true),
                              ALLOT_U32(&end_off, "end_offset", 10, true),
                              ALLOT_I32(&pos_inc, "pos_inc", 7, false),
                              ALLOT_F32(&boost, "boost", 5, false),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
    }

    STRLEN      len;
    char       *text = SvPVutf8(text_sv, len);
    lucy_Token *self = (lucy_Token*)XSBind_new_blank_obj(either_sv);
    lucy_Token_init(self, text, len, start_off, end_off, boost,
                    pos_inc);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

SV*
get_text(self)
    lucy_Token *self;
CODE:
    RETVAL = newSVpvn(Lucy_Token_Get_Text(self), Lucy_Token_Get_Len(self));
    SvUTF8_on(RETVAL);
OUTPUT: RETVAL

void
set_text(self, sv)
    lucy_Token *self;
    SV *sv;
PPCODE:
{
    STRLEN len;
    char *ptr = SvPVutf8(sv, len);
    Lucy_Token_Set_Text(self, ptr, len);
}
END_XS

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Analysis::Token",
    bind_methods => [
        qw(
            Get_Start_Offset
            Get_End_Offset
            Get_Boost
            Get_Pos_Inc
            )
    ],
    xs_code => $xs,
);

