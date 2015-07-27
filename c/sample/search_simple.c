#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/String.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Simple.h"

const char path_to_index[] = "lucy_index";

static void
S_usage_and_exit(const char *arg0) {
    printf("Usage: %s <querystring>\n", arg0);
    exit(1);
}

int
main(int argc, char *argv[]) {
    // Initialize the library.
    lucy_bootstrap_parcel();

    if (argc != 2) {
        S_usage_and_exit(argv[0]);
    }

    const char *query_c = argv[1];

    printf("Searching for: %s\n\n", query_c);

    String *folder   = Str_newf("%s", path_to_index);
    String *language = Str_newf("en");
    Simple *lucy     = Simple_new((Obj*)folder, language);

    String *query_str = Str_newf("%s", query_c);
    Simple_Search(lucy, query_str, 0, 10);

    String *title_str = Str_newf("title");
    String *url_str   = Str_newf("url");
    HitDoc *hit;
    int i = 1;

    // Loop over search results.
    while (NULL != (hit = Simple_Next(lucy))) {
        String *title = (String*)HitDoc_Extract(hit, title_str);
        char *title_c = Str_To_Utf8(title);

        String *url = (String*)HitDoc_Extract(hit, url_str);
        char *url_c = Str_To_Utf8(url);

        printf("Result %d: %s (%s)\n", i, title_c, url_c);

        free(url_c);
        free(title_c);
        DECREF(url);
        DECREF(title);
        DECREF(hit);
        i++;
    }

    DECREF(url_str);
    DECREF(title_str);
    DECREF(query_str);
    DECREF(lucy);
    DECREF(language);
    DECREF(folder);
    return 0;
}

