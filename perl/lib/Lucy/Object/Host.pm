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

package Lucy::Object::Host;
use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Object::Host

=for comment

These are all for testing purposes only.

=cut

IV
_test(...)
CODE:
    RETVAL = items;
OUTPUT: RETVAL

SV*
_test_obj(...)
CODE:
{
    lucy_ByteBuf *test_obj = lucy_BB_new_bytes("blah", 4);
    SV *pack_var = get_sv("Lucy::Object::Host::testobj", 1);
    RETVAL = (SV*)Lucy_BB_To_Host(test_obj);
    SvSetSV_nosteal(pack_var, RETVAL);
    LUCY_DECREF(test_obj);
    CHY_UNUSED_VAR(items);
}
OUTPUT: RETVAL

void
_callback(obj)
    lucy_Obj *obj;
PPCODE:
{
    lucy_ZombieCharBuf *blank = CFISH_ZCB_BLANK();
    lucy_Host_callback(obj, "_test", 2,
                       CFISH_ARG_OBJ("nothing", (lucy_CharBuf*)blank),
                       CFISH_ARG_I32("foo", 3));
}

int64_t
_callback_i64(obj)
    lucy_Obj *obj;
CODE:
{
    lucy_ZombieCharBuf *blank = CFISH_ZCB_BLANK();
    RETVAL
        = lucy_Host_callback_i64(obj, "_test", 2,
                                 CFISH_ARG_OBJ("nothing", (lucy_CharBuf*)blank),
                                 CFISH_ARG_I32("foo", 3));
}
OUTPUT: RETVAL

double
_callback_f64(obj)
    lucy_Obj *obj;
CODE:
{
    lucy_ZombieCharBuf *blank = CFISH_ZCB_BLANK();
    RETVAL
        = lucy_Host_callback_f64(obj, "_test", 2,
                                 CFISH_ARG_OBJ("nothing", (lucy_CharBuf*)blank),
                                 CFISH_ARG_I32("foo", 3));
}
OUTPUT: RETVAL

SV*
_callback_obj(obj)
    lucy_Obj *obj;
CODE:
{
    lucy_Obj *other = lucy_Host_callback_obj(obj, "_test_obj", 0);
    RETVAL = (SV*)Lucy_Obj_To_Host(other);
    LUCY_DECREF(other);
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel     => "Lucy",
    class_name => "Lucy::Object::Host",
    xs_code    => $xs_code,
);


