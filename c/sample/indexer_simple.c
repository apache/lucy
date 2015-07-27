#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/String.h"
#include "Lucy/Simple.h"
#include "Lucy/Document/Doc.h"

const char path_to_index[] = "lucy_index";
const char uscon_source[]  = "../../common/sample/us_constitution";

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

    String *folder   = Str_newf("%s", path_to_index);
    String *language = Str_newf("en");
    Simple *lucy     = Simple_new((Obj*)folder, language);

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
            Simple_Add_Doc(lucy, doc); // ta-da!
            DECREF(doc);
        }
    }

    closedir(dir);

    DECREF(lucy);
    DECREF(language);
    DECREF(folder);
    return 0;
}

