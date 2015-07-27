#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/String.h"
#include "Lucy/Analysis/EasyAnalyzer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/StringType.h"
#include "Lucy/Plan/Schema.h"

const char path_to_index[] = "lucy_index";
const char uscon_source[]  = "../../common/sample/us_constitution";

static Schema*
S_create_schema() {
    // Create a new schema.
    Schema *schema = Schema_new();

    // Create an analyzer.
    String       *language = Str_newf("en");
    EasyAnalyzer *analyzer = EasyAnalyzer_new(language);

    // Specify fields.

    {
        String *field_str = Str_newf("title");
        FullTextType *type = FullTextType_new((Analyzer*)analyzer);
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(type);
        DECREF(field_str);
    }

    {
        String *field_str = Str_newf("content");
        FullTextType *type = FullTextType_new((Analyzer*)analyzer);
        FullTextType_Set_Highlightable(type, true);
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(type);
        DECREF(field_str);
    }

    {
        String *field_str = Str_newf("url");
        StringType *type = StringType_new();
        StringType_Set_Indexed(type, false);
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(type);
        DECREF(field_str);
    }

    {
        String *field_str = Str_newf("category");
        StringType *type = StringType_new();
        StringType_Set_Stored(type, false);
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(type);
        DECREF(field_str);
    }

    DECREF(analyzer);
    DECREF(language);
    return schema;
}

bool
S_starts_with(const char *str, const char *prefix) {
    size_t len        = strlen(str);
    size_t prefix_len = strlen(prefix);

    return len >= prefix_len
           && memcmp(str, prefix, prefix_len) == 0;
}

bool
S_ends_with(const char *str, const char *postfix) {
    size_t len         = strlen(str);
    size_t postfix_len = strlen(postfix);

    return len >= postfix_len
           && memcmp(str + len - postfix_len, postfix, postfix_len) == 0;
}

Doc*
S_parse_file(const char *filename) {
    size_t bytes = strlen(uscon_source) + 1 + strlen(filename) + 1;
    char *path = (char*)malloc(bytes);
    path[0] = '\0';
    strcat(path, uscon_source);
    strcat(path, "/");
    strcat(path, filename);

    FILE *stream = fopen(path, "r");
    if (stream == NULL) {
        perror(path);
        exit(1);
    }

    char *title    = NULL;
    char *bodytext = NULL;
    if (fscanf(stream, "%m[^\r\n] %m[\x01-\x7F]", &title, &bodytext) != 2) {
        fprintf(stderr, "Can't extract title/bodytext from '%s'", path);
        exit(1);
    }

    const char *category = NULL;
    if (S_starts_with(filename, "art")) {
        category = "article";
    }
    else if (S_starts_with(filename, "amend")) {
        category = "amendment";
    }
    else if (S_starts_with(filename, "preamble")) {
        category = "preamble";
    }
    else {
        fprintf(stderr, "Can't derive category for %s", filename);
        exit(1);
    }

    Doc *doc = Doc_new(NULL, 0);

    {
        // Store 'title' field
        String *field = Str_newf("title");
        String *value = Str_new_from_utf8(title, strlen(title));
        Doc_Store(doc, field, (Obj*)value);
        DECREF(field);
        DECREF(value);
    }

    {
        // Store 'content' field
        String *field = Str_newf("content");
        String *value = Str_new_from_utf8(bodytext, strlen(bodytext));
        Doc_Store(doc, field, (Obj*)value);
        DECREF(field);
        DECREF(value);
    }

    {
        // Store 'url' field
        String *field = Str_newf("url");
        String *value = Str_new_from_utf8(filename, strlen(filename));
        Doc_Store(doc, field, (Obj*)value);
        DECREF(field);
        DECREF(value);
    }

    {
        // Store 'category' field
        String *field = Str_newf("category");
        String *value = Str_new_from_utf8(category, strlen(category));
        Doc_Store(doc, field, (Obj*)value);
        DECREF(field);
        DECREF(value);
    }

    fclose(stream);
    free(bodytext);
    free(title);
    free(path);
    return doc;
}

int
main() {
    // Initialize the library.
    lucy_bootstrap_parcel();

    Schema *schema = S_create_schema();
    String *folder = Str_newf("%s", path_to_index);

    Indexer *indexer = Indexer_new(schema, (Obj*)folder, NULL,
                                   Indexer_CREATE | Indexer_TRUNCATE);

    DIR *dir = opendir(uscon_source);
    if (dir == NULL) {
        perror(uscon_source);
        return 1;
    }

    for (struct dirent *entry = readdir(dir);
         entry;
         entry = readdir(dir)) {

        if (S_ends_with(entry->d_name, ".txt")) {
            Doc *doc = S_parse_file(entry->d_name);
            Indexer_Add_Doc(indexer, doc, 1.0);
            DECREF(doc);
        }
    }

    closedir(dir);

    Indexer_Commit(indexer);

    DECREF(indexer);
    DECREF(folder);
    DECREF(schema);
    return 0;
}

