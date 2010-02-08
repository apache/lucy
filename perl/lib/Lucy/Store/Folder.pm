package Lucy::Store::Folder;
use Lucy;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Store::Folder",
    bind_methods => [
        qw(
            Open_Out
            Open_In
            MkDir
            List_R
            Exists
            Rename
            Hard_Link
            Delete
            Slurp_File
            Close
            Get_Path
            )
    ],
    bind_constructors => ["new"],
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

