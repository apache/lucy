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

/*
 * Sample program to get started with the Apache Lucy C library.
 *
 * Creates an index with a few documents and conducts a few searches.
 *
 * If $PREFIX points to your installation directory, it can be compiled with:
 *
 *     $PREFIX/bin/cfc \
 *         --include=$PREFIX/share/clownfish/include \
 *         --parcel=Lucy \
 *         --dest=autogen
 *     c99 \
 *         getting_started.c \
 *         -I autogen/include -L $PREFIX/lib -l clownfish -l lucy \
 *         -o getting_started
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/String.h"
#include "Lucy/Analysis/EasyAnalyzer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"

static Schema*
S_create_schema();

static void
S_index_documents(Schema *schema, String *folder);

static void
S_add_document(Indexer *indexer, const char *title, const char *content);

static void
S_search(IndexSearcher *searcher, const char *query);

int
main() {
    // Initialize the library.
    lucy_bootstrap_parcel();

    Schema *schema = S_create_schema();
    String *folder = Str_newf("lucy_index");

    S_index_documents(schema, folder);

    IndexSearcher *searcher = IxSearcher_new((Obj*)folder);

    S_search(searcher, "ullamco");
    S_search(searcher, "ut OR laborum");
    S_search(searcher, "\"fugiat nulla\"");

    DECREF(schema);
    DECREF(folder);
    DECREF(searcher);
    return 0;
}

static Schema*
S_create_schema() {
    // Create a new schema.
    Schema *schema = Schema_new();

    // Create an analyzer.
    String       *language = Str_newf("en");
    EasyAnalyzer *analyzer = EasyAnalyzer_new(language);

    // Specify fields.

    FullTextType *type = FullTextType_new((Analyzer*)analyzer);

    {
        String *field_str = Str_newf("title");
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(field_str);
    }

    {
        String *field_str = Str_newf("content");
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(field_str);
    }

    DECREF(language);
    DECREF(analyzer);
    DECREF(type);
    return schema;
}

static void
S_index_documents(Schema *schema, String *folder) {
    Indexer *indexer = Indexer_new(schema, (Obj*)folder, NULL,
                                   Indexer_CREATE | Indexer_TRUNCATE);

    S_add_document(indexer, "Lorem ipsum",
        "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do"
        " eiusmod tempor incididunt ut labore et dolore magna aliqua."
    );
    S_add_document(indexer, "Ut enim",
        "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris"
        " nisi ut aliquip ex ea commodo consequat."
    );
    S_add_document(indexer, "Duis aute",
        "Duis aute irure dolor in reprehenderit in voluptate velit essei"
        " cillum dolore eu fugiat nulla pariatur."
    );
    S_add_document(indexer, "Excepteur sint",
        "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui"
        " officia deserunt mollit anim id est laborum."
    );

    Indexer_Commit(indexer);

    DECREF(indexer);
}

static void
S_add_document(Indexer *indexer, const char *title, const char *content) {
    Doc *doc = Doc_new(NULL, 0);

    {
        // Store 'title' field   
        String *field_str = Str_newf("title");
        String *value_str = Str_new_from_utf8(title, strlen(title));
        Doc_Store(doc, field_str, (Obj*)value_str);
        DECREF(field_str);
        DECREF(value_str);
    }

    {
        // Store 'content' field   
        String *field_str = Str_newf("content");
        String *value_str = Str_new_from_utf8(content, strlen(content));
        Doc_Store(doc, field_str, (Obj*)value_str);
        DECREF(field_str);
        DECREF(value_str);
    }

    Indexer_Add_Doc(indexer, doc, 1.0);

    DECREF(doc);
}

static void
S_search(IndexSearcher *searcher, const char *query) {
    printf("Searching for: %s\n", query);

    // Execute search query.
    String *query_str = Str_new_from_utf8(query, strlen(query));
    Hits   *hits      = IxSearcher_Hits(searcher, (Obj*)query_str, 0, 10, NULL);

    String *field_str = Str_newf("title");
    HitDoc *hit;
    int i = 1;

    // Loop over search results.
    while (NULL != (hit = Hits_Next(hits))) {
        String *value_str = (String*)HitDoc_Extract(hit, field_str);
        char *value = Str_To_Utf8(value_str);

        printf("Result %d: %s\n", i, value);

        free(value);
        DECREF(value_str);
        DECREF(hit);
        i++;
    }

    printf("\n");

    DECREF(query_str);
    DECREF(hits);
    DECREF(field_str);
}

