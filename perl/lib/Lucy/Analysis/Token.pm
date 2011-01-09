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
    SV *text_sv         = NULL;
    SV *start_offset_sv = NULL;
    SV *end_offset_sv   = NULL;
    SV *pos_inc_sv      = NULL;
    SV *boost_sv        = NULL;

    chy_bool_t args_ok = XSBind_allot_params(
        &(ST(0)), 1, items, "Lucy::Analysis::Token::new_PARAMS",
        &text_sv, "text", 4,
        &start_offset_sv, "start_offset", 12, 
        &end_offset_sv, "end_offset", 10, 
        &pos_inc_sv, "pos_inc", 7, 
        &boost_sv, "boost", 5, 
        NULL);
    if (!args_ok) {
        CFISH_RETHROW(LUCY_INCREF(cfish_Err_get_error()));
    }

    if (!XSBind_sv_defined(text_sv)) { 
        THROW(LUCY_ERR, "Missing required param 'text'"); 
    }
    if (!XSBind_sv_defined(start_offset_sv)) { 
        THROW(LUCY_ERR, "Missing required param 'start_offset'"); 
    }
    if (!XSBind_sv_defined(end_offset_sv)) { 
        THROW(LUCY_ERR, "Missing required param 'end_offset'"); 
    }

    STRLEN      len;
    char       *text      = SvPVutf8(text_sv, len);
    uint32_t    start_off = SvUV(start_offset_sv);
    uint32_t    end_off   = SvUV(end_offset_sv);
    int32_t     pos_inc   = pos_inc_sv ? SvIV(pos_inc_sv) : 1;
    float       boost     = boost_sv ? (float)SvNV(boost_sv) : 1.0f;
    lucy_Token *self   = (lucy_Token*)XSBind_new_blank_obj(either_sv);
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

Clownfish::Binding::Perl::Class->register(
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

