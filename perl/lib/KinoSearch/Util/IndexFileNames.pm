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

package KinoSearch::Util::IndexFileNames;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Util::IndexFileNames

uint64_t
extract_gen(name)
    const kino_CharBuf *name;
CODE:
    RETVAL = kino_IxFileNames_extract_gen(name);
OUTPUT: RETVAL

SV*
latest_snapshot(folder)
    kino_Folder *folder;
CODE:
{
    kino_CharBuf *latest = kino_IxFileNames_latest_snapshot(folder);
    RETVAL = XSBind_cb_to_sv(latest);   
    KINO_DECREF(latest);
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel     => "KinoSearch",
    class_name => "KinoSearch::Util::IndexFileNames",
    xs_code    => $xs_code,
);


