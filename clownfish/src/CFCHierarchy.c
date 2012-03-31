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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#ifndef true
    #define true 1
    #define false 0
#endif

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCHierarchy.h"
#include "CFCClass.h"
#include "CFCFile.h"
#include "CFCFileSpec.h"
#include "CFCSymbol.h"
#include "CFCUtil.h"
#include "CFCParser.h"

struct CFCHierarchy {
    CFCBase base;
    size_t num_sources;
    char **sources;
    size_t num_includes;
    char **includes;
    char *dest;
    char *inc_dest;
    char *src_dest;
    CFCParser *parser;
    CFCClass **trees;
    size_t num_trees;
    CFCFile **files;
    size_t num_files;
    CFCClass **classes;
    size_t classes_cap;
    size_t num_classes;
};

static void
S_do_make_path(const char *path);

static void
S_parse_cf_files(CFCHierarchy *self, const char *source_dir, int is_included);

static void
S_connect_classes(CFCHierarchy *self);

static void
S_add_file(CFCHierarchy *self, CFCFile *file);

static void
S_add_tree(CFCHierarchy *self, CFCClass *klass);

static CFCFile*
S_fetch_file(CFCHierarchy *self, const char *path_part);

// Recursive helper function for CFCUtil_propagate_modified.
static int
S_do_propagate_modified(CFCHierarchy *self, CFCClass *klass, int modified);

const static CFCMeta CFCHIERARCHY_META = {
    "Clownfish::CFC::Model::Hierarchy",
    sizeof(CFCHierarchy),
    (CFCBase_destroy_t)CFCHierarchy_destroy
};

CFCHierarchy*
CFCHierarchy_new(const char *dest) {
    CFCHierarchy *self = (CFCHierarchy*)CFCBase_allocate(&CFCHIERARCHY_META);
    return CFCHierarchy_init(self, dest);
}

CFCHierarchy*
CFCHierarchy_init(CFCHierarchy *self, const char *dest) {
    if (!dest || !strlen(dest)) {
        CFCUtil_die("'dest' is required");
    }
    self->sources      = (char**)CALLOCATE(1, sizeof(char*));
    self->num_sources  = 0;
    self->includes     = (char**)CALLOCATE(1, sizeof(char*));
    self->num_includes = 0;
    self->dest         = CFCUtil_strdup(dest);
    self->trees        = (CFCClass**)CALLOCATE(1, sizeof(CFCClass*));
    self->num_trees    = 0;
    self->files        = (CFCFile**)CALLOCATE(1, sizeof(CFCFile*));
    self->num_files    = 0;
    self->classes_cap  = 10;
    self->classes      = (CFCClass**)CALLOCATE(
                            (self->classes_cap + 1), sizeof(CFCClass*));
    self->num_classes  = 0;
    self->parser       = CFCParser_new();

    self->inc_dest = CFCUtil_cat(CFCUtil_strdup(""), self->dest,
                                 CFCUTIL_PATH_SEP, "include", NULL);
    self->src_dest = CFCUtil_cat(CFCUtil_strdup(""), self->dest,
                                 CFCUTIL_PATH_SEP, "source", NULL);
    S_do_make_path(self->inc_dest);
    S_do_make_path(self->src_dest);

    return self;
}

void
CFCHierarchy_destroy(CFCHierarchy *self) {
    for (size_t i = 0; self->trees[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->trees[i]);
    }
    for (size_t i = 0; self->files[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->files[i]);
    }
    for (size_t i = 0; self->classes[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->classes[i]);
    }
    for (size_t i = 0; self->sources[i] != NULL; i++) {
        FREEMEM(self->sources[i]);
    }
    for (size_t i = 0; self->includes[i] != NULL; i++) {
        FREEMEM(self->includes[i]);
    }
    FREEMEM(self->trees);
    FREEMEM(self->files);
    FREEMEM(self->classes);
    FREEMEM(self->sources);
    FREEMEM(self->includes);
    FREEMEM(self->dest);
    FREEMEM(self->inc_dest);
    FREEMEM(self->src_dest);
    CFCBase_decref((CFCBase*)self->parser);
    CFCBase_destroy((CFCBase*)self);
}

static void
S_do_make_path(const char *path) {
    if (!CFCUtil_is_dir(path)) {
        CFCUtil_make_path(path);
        if (!CFCUtil_is_dir(path)) {
            CFCUtil_die("Can't make path %s", path);
        }
    }
}

void
CFCHierarchy_add_source_dir(CFCHierarchy *self, const char *source_dir) {
    size_t n = self->num_sources;
    self->sources      = (char**)REALLOCATE(self->sources, n + 2);
    self->sources[n]   = CFCUtil_strdup(source_dir);
    self->sources[n+1] = NULL;
    self->num_sources  = n + 1;
}

void
CFCHierarchy_add_include_dir(CFCHierarchy *self, const char *include_dir) {
    size_t n = self->num_includes;
    self->includes      = (char**)REALLOCATE(self->includes, n + 2);
    self->includes[n]   = CFCUtil_strdup(include_dir);
    self->includes[n+1] = NULL;
    self->num_includes  = n + 1;
}

void
CFCHierarchy_build(CFCHierarchy *self) {
    for (size_t i = 0; self->sources[i] != NULL; i++) {
        S_parse_cf_files(self, self->sources[i], 0);
    }
    for (size_t i = 0; self->includes[i] != NULL; i++) {
        S_parse_cf_files(self, self->includes[i], 1);
    }
    S_connect_classes(self);
    for (size_t i = 0; self->trees[i] != NULL; i++) {
        CFCClass_grow_tree(self->trees[i]);
    }
}

static char**
S_find_cfh(const char *dir, char **cfh_list, size_t num_cfh) {
    void *dirhandle = CFCUtil_opendir(dir);
    size_t full_path_cap = strlen(dir) * 2;
    char *full_path = (char*)MALLOCATE(full_path_cap);
    const char *entry = NULL;
    while (NULL != (entry = CFCUtil_dirnext(dirhandle))) {
        // Ignore updirs and hidden files.
        if (strncmp(entry, ".", 1) == 0) {
            continue;
        }

        size_t name_len = strlen(entry);
        size_t needed = strlen(dir) + 1 + name_len + 1;
        if (needed > full_path_cap) {
            full_path_cap = needed;
            full_path = (char*)REALLOCATE(full_path, full_path_cap);
        }
        int full_path_len = sprintf(full_path, "%s" CFCUTIL_PATH_SEP "%s",
                                    dir, entry);
        const char *cfh_suffix = strstr(full_path, ".cfh");

        if (cfh_suffix == full_path + (full_path_len - 4)) {
            cfh_list = (char**)REALLOCATE(cfh_list,
                                          (num_cfh + 2) * sizeof(char*));
            cfh_list[num_cfh++] = CFCUtil_strdup(full_path);
            cfh_list[num_cfh] = NULL;
        }
        else if (CFCUtil_is_dir(full_path)) {
            cfh_list = S_find_cfh(full_path, cfh_list, num_cfh);
            num_cfh = 0;
            if (cfh_list) {
                while (cfh_list[num_cfh] != NULL) { num_cfh++; }
            }
        }
    }

    FREEMEM(full_path);
    CFCUtil_closedir(dirhandle, dir);
    return cfh_list;
}

static void
S_parse_cf_files(CFCHierarchy *self, const char *source_dir, int is_included) {
    char **all_source_paths = (char**)CALLOCATE(1, sizeof(char*));
    all_source_paths = S_find_cfh(source_dir, all_source_paths, 0);
    size_t source_dir_len  = strlen(source_dir);
    char *path_part = NULL;
    size_t path_part_max = 0;

    // Process any file that has at least one class declaration.
    for (int i = 0; all_source_paths[i] != NULL; i++) {
        // Derive the name of the class that owns the module file.
        char *source_path = all_source_paths[i];
        size_t source_path_len = strlen(source_path);
        if (strncmp(source_path, source_dir, source_dir_len) != 0) {
            CFCUtil_die("'%s' doesn't start with '%s'", source_path,
                        source_dir);
        }
        size_t path_part_len = source_path_len
                               - source_dir_len
                               - strlen(".cfh");
        if (path_part_max < path_part_len + 1) {
            path_part_max = path_part_len + 1;
            path_part = (char*)REALLOCATE(path_part, path_part_max);
        }
        const char *src = source_path + source_dir_len;
        while (*src == CFCUTIL_PATH_SEP_CHAR) {
            ++src;
            --path_part_len;
        }
        memcpy(path_part, src, path_part_len);
        path_part[path_part_len] = '\0';

        CFCFileSpec *file_spec = CFCFileSpec_new(source_dir, path_part,
                                                 is_included);

        // Slurp, parse, add parsed file to pool.
        size_t unused;
        char *content = CFCUtil_slurp_text(source_path, &unused);
        CFCFile *file = CFCParser_parse_file(self->parser, content, file_spec);
        FREEMEM(content);
        if (!file) {
            CFCUtil_die("parser error for %s", source_path);
        }
        S_add_file(self, file);

        CFCClass **classes_in_file = CFCFile_classes(file);
        for (size_t j = 0; classes_in_file[j] != NULL; j++) {
            if (self->num_classes == self->classes_cap) {
                self->classes_cap += 10;
                self->classes = (CFCClass**)REALLOCATE(
                                  self->classes,
                                  (self->classes_cap + 1) * sizeof(CFCClass*));
            }
            self->classes[self->num_classes++]
                = (CFCClass*)CFCBase_incref((CFCBase*)classes_in_file[j]);
        }
        CFCBase_decref((CFCBase*)file);
        CFCBase_decref((CFCBase*)file_spec);
    }
    self->classes[self->num_classes] = NULL;

    for (int i = 0; all_source_paths[i] != NULL; i++) {
        FREEMEM(all_source_paths[i]);
    }
    FREEMEM(all_source_paths);
    FREEMEM(path_part);
}

static void
S_connect_classes(CFCHierarchy *self) {
    // Wrangle the classes into hierarchies and figure out inheritance.
    for (int i = 0; self->classes[i] != NULL; i++) {
        CFCClass *klass = self->classes[i];
        const char *parent_name = CFCClass_get_parent_class_name(klass);
        if (parent_name) {
            for (size_t j = 0; ; j++) {
                CFCClass *maybe_parent = self->classes[j];
                if (!maybe_parent) {
                    CFCUtil_die("Parent class '%s' not defined", parent_name);
                }
                const char *maybe_parent_name
                    = CFCClass_get_class_name(maybe_parent);
                if (strcmp(parent_name, maybe_parent_name) == 0) {
                    CFCClass_add_child(maybe_parent, klass);
                    break;
                }
            }
        }
        else {
            S_add_tree(self, klass);
        }
    }
}

int
CFCHierarchy_propagate_modified(CFCHierarchy *self, int modified) {
    // Seed the recursive write.
    int somebody_is_modified = false;
    for (size_t i = 0; self->trees[i] != NULL; i++) {
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
S_do_propagate_modified(CFCHierarchy *self, CFCClass *klass, int modified) {
    const char *path_part = CFCClass_get_path_part(klass);
    CFCUTIL_NULL_CHECK(path_part);
    CFCFile *file = S_fetch_file(self, path_part);
    CFCUTIL_NULL_CHECK(file);
    const char *source_dir = CFCFile_get_source_dir(file);
    CFCUTIL_NULL_CHECK(source_dir);
    size_t cfh_buf_size = CFCFile_path_buf_size(file, source_dir);
    char *source_path = (char*)MALLOCATE(cfh_buf_size);
    CFCFile_cfh_path(file, source_path, cfh_buf_size, source_dir);
    size_t h_buf_size = CFCFile_path_buf_size(file, self->dest);
    char *h_path = (char*)MALLOCATE(h_buf_size);
    CFCFile_h_path(file, h_path, h_buf_size, self->dest);

    if (!CFCUtil_current(source_path, h_path)) {
        modified = true;
    }
    FREEMEM(h_path);
    FREEMEM(source_path);
    if (modified) {
        CFCFile_set_modified(file, modified);
    }

    // Proceed to the next generation.
    int somebody_is_modified = modified;
    CFCClass **children = CFCClass_children(klass);
    for (size_t i = 0; children[i] != NULL; i++) {
        CFCClass *kid = children[i];
        if (CFCClass_final(klass)) {
            CFCUtil_die("Attempt to inherit from final class '%s' by '%s'",
                        CFCClass_get_class_name(klass),
                        CFCClass_get_class_name(kid));
        }
        if (S_do_propagate_modified(self, kid, modified)) {
            somebody_is_modified = 1;
        }
    }

    return somebody_is_modified;
}

static void
S_add_tree(CFCHierarchy *self, CFCClass *klass) {
    CFCUTIL_NULL_CHECK(klass);
    const char *full_struct_sym = CFCClass_full_struct_sym(klass);
    for (size_t i = 0; self->trees[i] != NULL; i++) {
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
CFCHierarchy_ordered_classes(CFCHierarchy *self) {
    size_t num_classes = 0;
    size_t max_classes = 10;
    CFCClass **ladder = (CFCClass**)MALLOCATE(
                            (max_classes + 1) * sizeof(CFCClass*));
    for (size_t i = 0; self->trees[i] != NULL; i++) {
        CFCClass *tree = self->trees[i];
        CFCClass **child_ladder = CFCClass_tree_to_ladder(tree);
        for (size_t j = 0; child_ladder[j] != NULL; j++) {
            if (num_classes == max_classes) {
                max_classes += 10;
                ladder = (CFCClass**)REALLOCATE(
                             ladder, (max_classes + 1) * sizeof(CFCClass*));
            }
            ladder[num_classes++] = child_ladder[j];
        }
        FREEMEM(child_ladder);
    }
    ladder[num_classes] = NULL;
    return ladder;
}

static CFCFile*
S_fetch_file(CFCHierarchy *self, const char *path_part) {
    for (size_t i = 0; self->files[i] != NULL; i++) {
        const char *existing = CFCFile_get_path_part(self->files[i]);
        if (strcmp(path_part, existing) == 0) {
            return self->files[i];
        }
    }
    return NULL;
}

static void
S_add_file(CFCHierarchy *self, CFCFile *file) {
    CFCUTIL_NULL_CHECK(file);
    const char *path_part = CFCFile_get_path_part(file);
    CFCClass **classes = CFCFile_classes(file);
    for (size_t i = 0; self->files[i] != NULL; i++) {
        CFCFile *existing = self->files[i];
        const char *old_path_part = CFCFile_get_path_part(existing);
        if (strcmp(path_part, old_path_part) == 0) {
            CFCUtil_die("File %s.cfh already registered",
                        path_part);
        }
        CFCClass **existing_classes = CFCFile_classes(existing);
        for (size_t j = 0; classes[j] != NULL; j++) {
            const char *new_class_name = CFCClass_get_class_name(classes[j]);
            for (size_t k = 0; existing_classes[k] != NULL; k++) {
                const char *existing_class_name
                    = CFCClass_get_class_name(existing_classes[k]);
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
CFCHierarchy_files(CFCHierarchy *self) {
    return self->files;
}

const char*
CFCHierarchy_get_source_dir(CFCHierarchy *self, size_t i) {
    return i < self->num_sources ? self->sources[i] : NULL;
}

size_t
CFCHierarchy_get_num_source_dirs(CFCHierarchy *self) {
    return self->num_sources;
}

const char*
CFCHierarchy_get_include_dir(CFCHierarchy *self, size_t i) {
    return i < self->num_includes ? self->includes[i] : NULL;
}

size_t
CFCHierarchy_get_num_include_dirs(CFCHierarchy *self) {
    return self->num_includes;
}

const char*
CFCHierarchy_get_dest(CFCHierarchy *self) {
    return self->dest;
}

const char*
CFCHierarchy_get_include_dest(CFCHierarchy *self) {
    return self->inc_dest;
}

const char*
CFCHierarchy_get_source_dest(CFCHierarchy *self) {
    return self->src_dest;
}


