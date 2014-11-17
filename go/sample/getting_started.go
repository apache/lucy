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
 * Sample program to get started with the Go bindings for the Apache Lucy C
 * library.
 *
 * Creates an index with a few documents and conducts a few searches.
 */

package main

import "git-wip-us.apache.org/repos/asf/lucy.git/go/lucy"
import "fmt"
import "log"

func main() {
	schema := createSchema()
	index := "lucydemo"
	indexDocuments(schema, index)
	searcher, err := lucy.OpenIndexSearcher(index)
	if err != nil {
		log.Fatal(err)
	}
	defer searcher.Close()
	performSearch(searcher, "ullamco")
	performSearch(searcher, "ut OR laborum")
	performSearch(searcher, `"fugiat nulla"`)
}

func createSchema() *lucy.Schema {
	// Create a new schema.
	schema := lucy.NewSchema()

	// Create an analyzer.
	analyzer := lucy.NewEasyAnalyzer("en")

	// Specify fields.
	fieldType := lucy.NewFullTextType(analyzer)
	schema.SpecField("title", fieldType)
	schema.SpecField("content", fieldType)

	return schema
}

type MyDoc struct {
	Title   string
	Content string
}

var docs []MyDoc = []MyDoc{
	MyDoc{
		Title: `lorem ipsum`,
		Content: `Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do
                  eiusmod tempor incididunt ut labore et dolore magna aliqua.`,
	},
	MyDoc{
		Title: `Ut enim`,
		Content: `Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris
                  nisi ut aliquip ex ea commodo consequat.`,
	},
	MyDoc{
		Title: `Duis aute`,
		Content: `Duis aute irure dolor in reprehenderit in voluptate velit essei
                  cillum dolore eu fugiat nulla pariatur.`,
	},
	MyDoc{
		Title: `Excepteur sint`,
		Content: `Excepteur sint occaecat cupidatat non proident, sunt in culpa qui
                  officia deserunt mollit anim id est laborum.`,
	},
}

func indexDocuments(schema *lucy.Schema, index string) {
	indexerArgs := &lucy.OpenIndexerArgs{
		Schema:   schema,
		Index:    index,
		Create:   true,
		Truncate: true,
	}
	indexer, err := lucy.OpenIndexer(indexerArgs)
	if err != nil {
		log.Fatal(err)
	}
	defer indexer.Close()

	for _, doc := range docs {
		err := indexer.AddDoc(&doc)
		if err != nil {
			log.Fatal(err)
		}
	}
	err = indexer.Commit()
	if err != nil {
		log.Fatal(err)
	}
}

func performSearch(searcher lucy.Searcher, query string) {
	fmt.Println("Searching for:", query)

	hits, err := searcher.Hits(query, 0, 10, nil)
	if err != nil {
		log.Fatal(err)
	}
	var hit MyDoc
	for hits.Next(&hit) {
		fmt.Printf("  Result: %s\n", hit.Title)
	}
	if err := hits.Error(); err != nil {
		log.Fatal(err)
	}
	fmt.Println()
}
