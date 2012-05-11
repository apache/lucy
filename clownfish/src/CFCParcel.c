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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#ifndef true
  #define true 1
  #define false 0
#endif

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCParcel.h"
#include "CFCUtil.h"

struct CFCParcel {
    CFCBase base;
    char *name;
    char *cnick;
    char *prefix;
    char *Prefix;
    char *PREFIX;
};

#define JSON_STRING 1
#define JSON_HASH   2

typedef struct JSONNode {
    int type;
    char *string;
    struct JSONNode **kids;
    size_t num_kids;
} JSONNode;

static JSONNode*
S_parse_json_for_parcel(const char *json);

static JSONNode*
S_parse_json_hash(const char **json);

static JSONNode*
S_parse_json_string(const char **json);

static void
S_skip_whitespace(const char **json);

static void
S_destroy_json(JSONNode *node);

#define MAX_PARCELS 100
static CFCParcel *registry[MAX_PARCELS + 1];
static int first_time = true;

CFCParcel*
CFCParcel_singleton(const char *name, const char *cnick) {
    // Set up registry.
    if (first_time) {
        for (size_t i = 1; i < MAX_PARCELS; i++) { registry[i] = NULL; }
        first_time = false;
    }

    // Return the default parcel for either a blank name or a NULL name.
    if (!name || !strlen(name)) {
        return CFCParcel_default_parcel();
    }

    // Return an existing singleton if the parcel has already been registered.
    size_t i;
    for (i = 1; registry[i] != NULL; i++) {
        CFCParcel *existing = registry[i];
        if (strcmp(existing->name, name) == 0) {
            if (cnick && strcmp(existing->cnick, cnick) != 0) {
                CFCUtil_die("cnick '%s' for parcel '%s' conflicts with '%s'",
                            cnick, name, existing->cnick);
            }
            return existing;
        }
    }
    if (i == MAX_PARCELS) {
        CFCUtil_die("Exceeded maximum number of parcels (%d)", MAX_PARCELS);
    }

    // Register new parcel.
    CFCParcel *singleton = CFCParcel_new(name, cnick);
    registry[i] = singleton;

    return singleton;
}

void
CFCParcel_reap_singletons(void) {
    if (registry[0]) {
        // default parcel.
        CFCBase_decref((CFCBase*)registry[0]);
    }
    for (int i = 1; registry[i] != NULL; i++) {
        CFCParcel *parcel = registry[i];
        CFCBase_decref((CFCBase*)parcel);
    }
}

static int
S_validate_name_or_cnick(const char *orig) {
    const char *ptr = orig;
    for (; *ptr != 0; ptr++) {
        if (!isalpha(*ptr)) { return false; }
    }
    return true;
}

const static CFCMeta CFCPARCEL_META = {
    "Clownfish::CFC::Model::Parcel",
    sizeof(CFCParcel),
    (CFCBase_destroy_t)CFCParcel_destroy
};

CFCParcel*
CFCParcel_new(const char *name, const char *cnick) {
    CFCParcel *self = (CFCParcel*)CFCBase_allocate(&CFCPARCEL_META);
    return CFCParcel_init(self, name, cnick);
}

CFCParcel*
CFCParcel_init(CFCParcel *self, const char *name, const char *cnick) {
    // Validate name.
    if (!name || !S_validate_name_or_cnick(name)) {
        CFCUtil_die("Invalid name: '%s'", name ? name : "[NULL]");
    }
    self->name = CFCUtil_strdup(name);

    // Validate or derive cnick.
    if (cnick) {
        if (!S_validate_name_or_cnick(cnick)) {
            CFCUtil_die("Invalid cnick: '%s'", cnick);
        }
        self->cnick = CFCUtil_strdup(cnick);
    }
    else {
        // Default cnick to name.
        self->cnick = CFCUtil_strdup(name);
    }

    // Derive prefix, Prefix, PREFIX.
    size_t cnick_len  = strlen(self->cnick);
    size_t prefix_len = cnick_len ? cnick_len + 1 : 0;
    size_t amount     = prefix_len + 1;
    self->prefix = (char*)MALLOCATE(amount);
    self->Prefix = (char*)MALLOCATE(amount);
    self->PREFIX = (char*)MALLOCATE(amount);
    memcpy(self->Prefix, self->cnick, cnick_len);
    if (cnick_len) {
        self->Prefix[cnick_len]  = '_';
        self->Prefix[cnick_len + 1]  = '\0';
    }
    else {
        self->Prefix[cnick_len] = '\0';
    }
    for (size_t i = 0; i < amount; i++) {
        self->prefix[i] = tolower(self->Prefix[i]);
        self->PREFIX[i] = toupper(self->Prefix[i]);
    }
    self->prefix[prefix_len] = '\0';
    self->Prefix[prefix_len] = '\0';
    self->PREFIX[prefix_len] = '\0';

    return self;
}

CFCParcel*
CFCParcel_new_from_json(const char *json) {
    JSONNode *parsed = S_parse_json_for_parcel(json);
    if (!parsed) {
        CFCUtil_die("Invalid JSON parcel definition");
    }
    const char *name     = NULL;
    const char *nickname = NULL;
    for (size_t i = 0, max = parsed->num_kids; i < max; i += 2) {
        JSONNode *key   = parsed->kids[i];
        JSONNode *value = parsed->kids[i + 1];
        if (key->type != JSON_STRING) {
            CFCUtil_die("JSON parsing error");
        }
        if (strcmp(key->string, "name") == 0) {
            if (value->type != JSON_STRING) {
                CFCUtil_die("'name' must be a string");
            }
            name = value->string;
        }
        else if (strcmp(key->string, "nickname") == 0) {
            if (value->type != JSON_STRING) {
                CFCUtil_die("'name' must be a string");
            }
            nickname = value->string;
        }
    }
    if (!name) {
        CFCUtil_die("Missing required key 'name'");
    }
    CFCParcel *self = CFCParcel_new(name, nickname);

    for (size_t i = 0, max = parsed->num_kids; i < max; i += 2) {
        JSONNode *key   = parsed->kids[i];
        JSONNode *value = parsed->kids[i + 1];
        if (strcmp(key->string, "name") == 0
            || strcmp(key->string, "nickname") == 0
           ) {
            ;
        }
        else {
            CFCUtil_die("Unrecognized key in parcel definition: '%s'",
                        key->string);
        }
    }

    S_destroy_json(parsed);
    return self;
}

void
CFCParcel_destroy(CFCParcel *self) {
    FREEMEM(self->name);
    FREEMEM(self->cnick);
    FREEMEM(self->prefix);
    FREEMEM(self->Prefix);
    FREEMEM(self->PREFIX);
    CFCBase_destroy((CFCBase*)self);
}

static CFCParcel *default_parcel = NULL;

CFCParcel*
CFCParcel_default_parcel(void) {
    if (default_parcel == NULL) {
        default_parcel = CFCParcel_new("DEFAULT", "");
        registry[0] = default_parcel;
    }
    return default_parcel;
}

CFCParcel*
CFCParcel_clownfish_parcel(void) {
    return CFCParcel_singleton("Lucy", "Lucy");
}

int
CFCParcel_equals(CFCParcel *self, CFCParcel *other) {
    if (strcmp(self->name, other->name)) { return false; }
    if (strcmp(self->cnick, other->cnick)) { return false; }
    return true;
}

const char*
CFCParcel_get_name(CFCParcel *self) {
    return self->name;
}

const char*
CFCParcel_get_cnick(CFCParcel *self) {
    return self->cnick;
}

const char*
CFCParcel_get_prefix(CFCParcel *self) {
    return self->prefix;
}

const char*
CFCParcel_get_Prefix(CFCParcel *self) {
    return self->Prefix;
}

const char*
CFCParcel_get_PREFIX(CFCParcel *self) {
    return self->PREFIX;
}

/*****************************************************************************
 * The hack JSON parser coded up below is only meant to parse Clownfish parcel
 * file content.  It is limited in its capabilities because so little is legal
 * in a .cfp file.
 */

static JSONNode*
S_parse_json_for_parcel(const char *json) {
    if (!json) {
        return NULL;
    }
    S_skip_whitespace(&json);
    if (*json != '{') {
        return NULL;
    }
    JSONNode *parsed = S_parse_json_hash(&json);
    S_skip_whitespace(&json);
    if (*json != '\0') {
        S_destroy_json(parsed);
        parsed = NULL;
    }
    return parsed;
}

static void
S_append_kid(JSONNode *node, JSONNode *child) {
    size_t size = (node->num_kids + 2) * sizeof(JSONNode*);
    node->kids = (JSONNode**)realloc(node->kids, size);
    node->kids[node->num_kids++] = child;
    node->kids[node->num_kids]   = NULL;
}

static JSONNode*
S_parse_json_hash(const char **json) {
    const char *text = *json;
    S_skip_whitespace(&text);
    if (*text != '{') {
        return NULL;
    }
    text++;
    JSONNode *node = (JSONNode*)calloc(1, sizeof(JSONNode));
    node->type = JSON_HASH;
    while (1) {
        // Parse key.
        S_skip_whitespace(&text);
        if (*text == '}') {
            text++;
            break;
        }
        else if (*text == '"') {
            JSONNode *key = S_parse_json_string(&text);
            S_skip_whitespace(&text);
            if (!key || *text != ':') {
                S_destroy_json(node);
                return NULL;
            }
            text++;
            S_append_kid(node, key);
        }
        else {
            S_destroy_json(node);
            return NULL;
        }

        // Parse value.
        S_skip_whitespace(&text);
        JSONNode *value = NULL;
        if (*text == '"') {
            value = S_parse_json_string(&text);
        }
        else if (*text == '{') {
            value = S_parse_json_hash(&text);
        }
        if (!value) {
            S_destroy_json(node);
            return NULL;
        }
        S_append_kid(node, value);

        // Parse comma.
        S_skip_whitespace(&text);
        if (*text == ',') {
            text++;
        }
        else if (*text == '}') {
            text++;
            break;
        }
        else {
            S_destroy_json(node);
            return NULL;
        }
    }

    // Move pointer.
    *json = text;

    return node;
}

// Parse a double quoted string.  Don't allow escapes.
static JSONNode*
S_parse_json_string(const char **json) {
    const char *text = *json; 
    if (*text != '\"') {
        return NULL;
    }
    text++;
    const char *start = text;
    while (*text != '"') {
        if (*text == '\\' || *text == '\0') {
            return NULL;
        }
        text++;
    }
    JSONNode *node = (JSONNode*)calloc(1, sizeof(JSONNode));
    node->type = JSON_STRING;
    node->string = CFCUtil_strndup(start, text - start);

    // Move pointer.
    text++;
    *json = text;

    return node;
}

static void
S_skip_whitespace(const char **json) {
    while (isspace(json[0][0])) { *json = *json + 1; }
}

static void
S_destroy_json(JSONNode *node) {
    if (!node) {
        return;
    }
    if (node->kids) {
        for (size_t i = 0; node->kids[i] != NULL; i++) {
            S_destroy_json(node->kids[i]);
        }
    }
    free(node->string);
    free(node->kids);
    free(node);
}

