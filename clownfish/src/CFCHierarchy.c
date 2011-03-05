/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifndef true
    #define true 1
    #define false 0
#endif

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCHierarchy.h"
#include "CFCClass.h"
#include "CFCFile.h"
#include "CFCSymbol.h"
#include "CFCUtil.h"

struct CFCHierarchy {
    CFCBase base;
    char *source;
    char *dest;
    CFCClass **trees;
    size_t num_trees;
    CFCFile **files;
    size_t num_files;
};

// Recursive helper function for CFCUtil_propagate_modified.
int
S_do_propagate_modified(CFCHierarchy *self, CFCClass *klass, int modified);

CFCHierarchy*
CFCHierarchy_new(const char *source, const char *dest)
{
    CFCHierarchy *self = (CFCHierarchy*)CFCBase_allocate(sizeof(CFCHierarchy),
        "Clownfish::Hierarchy");
    return CFCHierarchy_init(self, source, dest);
}

CFCHierarchy*
CFCHierarchy_init(CFCHierarchy *self, const char *source, const char *dest) 
{
    if (!source || !strlen(source) || !dest || !strlen(dest)) {
        croak("Both 'source' and 'dest' are required");
    }
    self->source    = CFCUtil_strdup(source);
    self->dest      = CFCUtil_strdup(dest);
    self->trees     = (CFCClass**)CALLOCATE(1, sizeof(CFCClass*));
    self->num_trees = 0;
    self->files     = (CFCFile**)CALLOCATE(1, sizeof(CFCFile*));
    self->num_files = 0;
    return self;
}

void
CFCHierarchy_destroy(CFCHierarchy *self)
{
    size_t i;
    for (i = 0; self->trees[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->trees[i]);
    }
    for (i = 0; self->files[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->files[i]);
    }
    FREEMEM(self->trees);
    FREEMEM(self->files);
    FREEMEM(self->source);
    FREEMEM(self->dest);
    CFCBase_destroy((CFCBase*)self);
}

CFCFile*
CFCHierarchy_parse_file(CFCHierarchy *self, void *parser, 
                        const char *content, const char *source_class)
{
    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    XPUSHs(sv_2mortal(newSVsv((SV*)parser)));
    XPUSHs(sv_2mortal(newSVpvn(content, strlen(content))));
    XPUSHs(sv_2mortal(newSVpvn(source_class, strlen(source_class))));
    PUTBACK;

    int count = call_pv("Clownfish::Hierarchy::_do_parse_file", G_SCALAR);

    SPAGAIN;

    if (count != 1) {
        CFCUtil_die("call to _do_parse_file failed\n");
    }

    SV *got = POPs;
    CFCFile *file = NULL;
    if (sv_derived_from(got, "Clownfish::File")) {
        IV tmp = SvIV(SvRV(got));
        file = INT2PTR(CFCFile*, tmp);
        CFCBase_incref((CFCBase*)file);
    }

    PUTBACK;
    FREETMPS;
    LEAVE;

    return file;
}

int
CFCHierarchy_propagate_modified(CFCHierarchy *self, int modified)
{
    // Seed the recursive write.
    int somebody_is_modified = false;
    size_t i;
    for (i = 0; self->trees[i] != NULL; i++) {
        CFCClass *tree = self->trees[i];
        if (S_do_propagate_modified(self, tree, modified)) {
            somebody_is_modified = true;
        }
    }
    if (somebody_is_modified || modified) { 
        return true; 
    }
    else {
        return false;
    }
}

int
S_do_propagate_modified(CFCHierarchy *self, CFCClass *klass, int modified)
{
    const char *source_class = CFCClass_get_source_class(klass);
    CFCFile *file = CFCHierarchy_fetch_file(self, source_class);
    size_t cfh_buf_size = CFCFile_path_buf_size(file, self->source);
    char *source_path = (char*)MALLOCATE(cfh_buf_size);
    CFCFile_cfh_path(file, source_path, cfh_buf_size, self->source);
    size_t h_buf_size = CFCFile_path_buf_size(file, self->dest);
    char *h_path = (char*)MALLOCATE(h_buf_size);
    CFCFile_h_path(file, h_path, h_buf_size, self->dest);

    if (!CFCUtil_current(source_path, h_path)) {
        modified = true;
    }
    if (modified) {
        CFCFile_set_modified(file, modified);
    }

    // Proceed to the next generation.
    int somebody_is_modified = modified;
    size_t i;
    CFCClass **children = CFCClass_children(klass);
    for (i = 0; children[i] != NULL; i++) {
        CFCClass *kid = children[i];
        if (CFCClass_final(klass)) {
            CFCUtil_die("Attempt to inherit from final class '%s' by '%s'",
                CFCSymbol_get_class_name((CFCSymbol*)klass),
                CFCSymbol_get_class_name((CFCSymbol*)kid));
        }
        if (S_do_propagate_modified(self, kid, modified)) {
            somebody_is_modified = 1;
        }
    }

    return somebody_is_modified;
}

void
CFCHierarchy_add_tree(CFCHierarchy *self, CFCClass *klass)
{
    CFCUTIL_NULL_CHECK(klass);
    const char *full_struct_sym = CFCClass_full_struct_sym(klass);
    size_t i;
    for (i = 0; self->trees[i] != NULL; i++) {
        const char *existing = CFCClass_full_struct_sym(self->trees[i]);
        if (strcmp(full_struct_sym, existing) == 0) {
            CFCUtil_die("Tree '%s' alread added", full_struct_sym);
        }
    }
    self->num_trees++;
    size_t size = (self->num_trees + 1) * sizeof(CFCClass*);
    self->trees = (CFCClass**)REALLOCATE(self->trees, size);
    self->trees[self->num_trees - 1] 
        = (CFCClass*)CFCBase_incref((CFCBase*)klass);
    self->trees[self->num_trees] = NULL;
}

CFCClass**
CFCHierarchy_trees(CFCHierarchy *self)
{
    return self->trees;
}

CFCClass**
CFCHierarchy_ordered_classes(CFCHierarchy *self)
{
    size_t num_classes = 0;
    size_t max_classes = 10;
    CFCClass **ladder = (CFCClass**)MALLOCATE((max_classes + 1) * sizeof(CFCClass*));
    size_t i;
    for (i = 0; self->trees[i] != NULL; i++) {
        CFCClass *tree = self->trees[i];
        CFCClass **child_ladder = CFCClass_tree_to_ladder(tree);
        size_t j;
        for (j = 0; child_ladder[j] != NULL; j++) {
            if (num_classes == max_classes) {
                max_classes += 10;
                ladder = (CFCClass**)REALLOCATE(ladder, 
                    (max_classes + 1) * sizeof(CFCClass*));
            }
            ladder[num_classes++] = child_ladder[j];
        }
        FREEMEM(child_ladder);
    }
    ladder[num_classes] = NULL;
    return ladder;
}

CFCFile*
CFCHierarchy_fetch_file(CFCHierarchy *self, const char *source_class)
{
    size_t i;
    for (i = 0; self->files[i] != NULL; i++) {
        const char *existing = CFCFile_get_source_class(self->files[i]);
        if (strcmp(source_class, existing) == 0) {
            return self->files[i];
        }
    }
    return NULL;
}

void
CFCHierarchy_add_file(CFCHierarchy *self, CFCFile *file)
{
    CFCUTIL_NULL_CHECK(file);
    const char *source_class = CFCFile_get_source_class(file);
    CFCClass **classes = CFCFile_classes(file);
    size_t i;
    for (i = 0; self->files[i] != NULL; i++) {
        CFCFile *existing = self->files[i];
        const char *old_source_class = CFCFile_get_source_class(existing);
        if (strcmp(source_class, old_source_class) == 0) {
            CFCUtil_die("File for source class %s already registered", 
                source_class);
        }
        CFCClass **existing_classes = CFCFile_classes(existing);
        size_t j;
        for (j = 0; classes[j] != NULL; j++) {
            const char *new_class_name
                = CFCSymbol_get_class_name((CFCSymbol*)classes[j]);
            size_t k; 
            for (k = 0; existing_classes[k] != NULL; k++) {
                const char *existing_class_name
                    = CFCSymbol_get_class_name((CFCSymbol*)existing_classes[k]);
                if (strcmp(new_class_name, existing_class_name) == 0) {
                    CFCUtil_die("Class '%s' already registered",
                        new_class_name);
                }
            }
        }
    }
    self->num_files++;
    size_t size = (self->num_files + 1) * sizeof(CFCFile*);
    self->files = (CFCFile**)REALLOCATE(self->files, size);
    self->files[self->num_files - 1] 
        = (CFCFile*)CFCBase_incref((CFCBase*)file);
    self->files[self->num_files] = NULL;
}

struct CFCFile**
CFCHierarchy_files(CFCHierarchy *self)
{
    return self->files;
}

const char*
CFCHierarchy_get_source(CFCHierarchy *self)
{
    return self->source;
}

const char*
CFCHierarchy_get_dest(CFCHierarchy *self)
{
    return self->dest;
}

