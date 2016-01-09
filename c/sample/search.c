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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/String.h"
#include "Clownfish/Vector.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Highlight/Highlighter.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/ANDQuery.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Search/TermQuery.h"
#include "Lucy/Search/QueryParser.h"

const char path_to_index[] = "lucy_index";

static void
S_usage_and_exit(const char *arg0) {
    printf("Usage: %s [-c <category>] <querystring>\n", arg0);
    exit(1);
}

int
main(int argc, char *argv[]) {
    // Initialize the library.
    lucy_bootstrap_parcel();

    const char *category = NULL;
    int i = 1;

    while (i < argc - 1) {
        if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 >= argc) {
                S_usage_and_exit(argv[0]);
            }
            i += 1;
            category = argv[i];
        }
        else {
            S_usage_and_exit(argv[0]);
        }

        i += 1;
    }

    if (i + 1 != argc) {
        S_usage_and_exit(argv[0]);
    }

    const char *query_c = argv[i];

    printf("Searching for: %s\n\n", query_c);

    String        *folder   = Str_newf("%s", path_to_index);
    IndexSearcher *searcher = IxSearcher_new((Obj*)folder);
    Schema        *schema   = IxSearcher_Get_Schema(searcher);
    QueryParser   *qparser  = QParser_new(schema, NULL, NULL, NULL);

    String *query_str = Str_newf("%s", query_c);
    Query  *query     = QParser_Parse(qparser, query_str);

    String *content_str = Str_newf("content");
    Highlighter *highlighter
        = Highlighter_new((Searcher*)searcher, (Obj*)query, content_str, 200);

    if (category) {
        String *category_name = Str_newf("category");
        String *category_str  = Str_newf("%s", category);
        TermQuery *category_query
            = TermQuery_new(category_name, (Obj*)category_str);

        Vector *children = Vec_new(2);
        Vec_Push(children, (Obj*)query);
        Vec_Push(children, (Obj*)category_query);
        query = (Query*)ANDQuery_new(children);

        DECREF(children);
        DECREF(category_str);
        DECREF(category_name);
    }

    Hits *hits = IxSearcher_Hits(searcher, (Obj*)query, 0, 10, NULL);

    String *title_str = Str_newf("title");
    String *url_str   = Str_newf("url");
    HitDoc *hit;
    i = 1;

    // Loop over search results.
    while (NULL != (hit = Hits_Next(hits))) {
        String *title = (String*)HitDoc_Extract(hit, title_str);
        char *title_c = Str_To_Utf8(title);

        String *url = (String*)HitDoc_Extract(hit, url_str);
        char *url_c = Str_To_Utf8(url);

        String *excerpt = Highlighter_Create_Excerpt(highlighter, hit);
        char *excerpt_c = Str_To_Utf8(excerpt);

        printf("Result %d: %s (%s)\n%s\n\n", i, title_c, url_c, excerpt_c);

        free(excerpt_c);
        free(url_c);
        free(title_c);
        DECREF(excerpt);
        DECREF(url);
        DECREF(title);
        DECREF(hit);
        i++;
    }

    DECREF(url_str);
    DECREF(title_str);
    DECREF(hits);
    DECREF(query);
    DECREF(query_str);
    DECREF(highlighter);
    DECREF(content_str);
    DECREF(qparser);
    DECREF(searcher);
    DECREF(folder);
    return 0;
}

