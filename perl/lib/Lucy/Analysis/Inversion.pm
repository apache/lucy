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

package Lucy::Analysis::Inversion;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $xs = <<'END_XS';
MODULE = Lucy   PACKAGE = Lucy::Analysis::Inversion

SV*
new(...)
CODE:
{
    lucy_Token *starter_token = NULL;
    // parse params, only if there's more than one arg
    if (items > 1) {
        SV *text_sv = NULL;
        chy_bool_t args_ok
            = XSBind_allot_params(&(ST(0)), 1, items,
                                  "Lucy::Analysis::Inversion::new_PARAMS",
                                  ALLOT_SV(&text_sv, "text", 4, false),
                                  NULL);
        if (!args_ok) {
            CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
        }
        if (XSBind_sv_defined(text_sv)) {
            STRLEN len;
            char *text = SvPVutf8(text_sv, len);
            starter_token = lucy_Token_new(text, len, 0, len, 1.0, 1);
        }
    }

    RETVAL = CFISH_OBJ_TO_SV_NOINC(lucy_Inversion_new(starter_token));
    CFISH_DECREF(starter_token);
}
OUTPUT: RETVAL
END_XS

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Analysis::Inversion",
    bind_methods => [qw( Append Reset Invert Next )],
    xs_code      => $xs,
);


