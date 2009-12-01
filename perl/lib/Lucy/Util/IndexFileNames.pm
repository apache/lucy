use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Util::IndexFileNames

IV
extract_gen(name)
    lucy_ZombieCharBuf name;
CODE:
    RETVAL = lucy_IxFileNames_extract_gen((lucy_CharBuf*)&name);
OUTPUT: RETVAL

SV*
latest_snapshot(folder)
    lucy_Folder *folder;
CODE:
{
    lucy_CharBuf *latest = lucy_IxFileNames_latest_snapshot(folder);
    RETVAL = XSBind_cb_to_sv(latest);   
    LUCY_DECREF(latest);
}
OUTPUT: RETVAL
END_XS_CODE

Boilerplater::Binding::Perl::Class->register(
    parcel     => "Lucy",
    class_name => "Lucy::Util::IndexFileNames",
    xs_code    => $xs_code,
);

__COPYRIGHT__

    /**
     * Copyright 2009 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

