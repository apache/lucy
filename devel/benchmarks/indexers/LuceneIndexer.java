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

import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.analysis.WhitespaceAnalyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;
import org.apache.lucene.store.FSDirectory;

import java.io.File;
import java.io.BufferedReader;
import java.io.FileReader;
import java.text.DecimalFormat;
import java.util.Date;
import java.util.Vector;
import java.util.Collections;
import java.util.Arrays;

/**
 * LuceneIndexer - benchmarking app
 * usage: java LuceneIndexer [-docs MAX_TO_INDEX] [-reps NUM_REPETITIONS]
 *
 * Recommended options: -server -Xmx500M -XX:CompileThreshold=100
 */

public class LuceneIndexer {
  static File corpusDir = new File("extracted_corpus");
  static File indexDir  = new File("lucene_index");
  static String[] fileList;

  public static void main (String[] args) throws Exception {
    // verify that we're running from the right directory
    if (!corpusDir.exists())
      throw new Exception("Can't find 'extracted_corpus' directory");

    // assemble the sorted list of article files
    fileList = buildFileList();

    // parse command line args
    int maxToIndex = fileList.length; // default: index all docs
    int numReps    = 1;               // default: run once
    int increment  = 0;
    boolean store  = false;
    String arg;
    int i = 0;
    while (i < (args.length - 1) && args[i].startsWith("-")) {
      arg = args[i++];
      if (arg.equals("-docs")) {
         maxToIndex = Integer.parseInt(args[i++]);
      }
      else if (arg.equals("-reps")) {
        numReps = Integer.parseInt(args[i++]);
      }
      else if (arg.equals("-increment")) {
        increment = Integer.parseInt(args[i++]);
      }
      else if (arg.equals("-store")) {
        if (Integer.parseInt(args[i++]) != 0) {
          store = true;
        }
      }
      else {
        throw new Exception("Unknown argument: " + arg);
      }
    }
    increment = increment == 0 ? maxToIndex + 1 : increment;

    // start the output
    System.out.println("---------------------------------------------------");

    // build the index numReps times, then print a final report
    float[] times = new float[numReps];
    for (int rep = 1; rep <= numReps; rep++) {
      // start the clock and build the index
      long start = new Date().getTime(); 
      int numIndexed = buildIndex(fileList, maxToIndex, increment, store);
  
      // stop the clock and print a report
      long end = new Date().getTime();
      float secs = (float)(end - start) / 1000;
      times[rep - 1] = secs;
      printInterimReport(rep, secs, numIndexed);
    }
    printFinalReport(times);
  }

  // Return a lexically sorted list of all article files from all subdirs.
  static String[] buildFileList () throws Exception {
    File[] articleDirs = corpusDir.listFiles();
    Vector<String> filePaths = new Vector<String>();
    for (int i = 0; i < articleDirs.length; i++) {
      File[] articles = articleDirs[i].listFiles();
      for (int j = 0; j < articles.length; j++) {
        String path = articles[j].getPath();
        if (path.indexOf("article") == -1)
          continue;
        filePaths.add(path);
      }
    }
    Collections.sort(filePaths);
    return (String[])filePaths.toArray(new String[filePaths.size()]);
  }

  // Initialize an IndexWriter
  static IndexWriter initWriter (int count) throws Exception {
    boolean create = count > 0 ? false : true;
    FSDirectory directory = FSDirectory.getDirectory(indexDir);
    IndexWriter writer = new IndexWriter(directory, true, 
        new WhitespaceAnalyzer(), create);
      // writer.setMaxBufferedDocs(1000);
      writer.setUseCompoundFile(false);
      
    return writer;
  }

  // Build an index, stopping at maxToIndex docs if maxToIndex > 0.
  static int buildIndex (String[] fileList, int maxToIndex, int increment,
                         boolean store) throws Exception {
    IndexWriter writer = initWriter(0);
    int docsSoFar = 0;

    while (docsSoFar < maxToIndex) {
      for (int i = 0; i < fileList.length; i++) {
        // add content to index
        File f = new File(fileList[i]);
        Document doc = new Document();
        BufferedReader br = new BufferedReader(new FileReader(f));
    
        try {
          // the title is the first line
          String title;
          if ( (title = br.readLine()) == null)
            throw new Exception("Failed to read title");
          Field titleField = new Field("title", title, Field.Store.YES, 
              Field.Index.TOKENIZED, Field.TermVector.NO);
          doc.add(titleField);
      
          // the body is the rest
          if (store) {
            StringBuffer buf = new StringBuffer();
            String str;
            while ( (str = br.readLine()) != null )
              buf.append( str );
            String body = buf.toString();
            Field bodyField = new Field("body", body, Field.Store.YES, 
                Field.Index.TOKENIZED, Field.TermVector.WITH_POSITIONS_OFFSETS);
            doc.add(bodyField);
          } else {
            Field bodyField = new Field("body", br);
            doc.add(bodyField);
          }

          writer.addDocument(doc);
        } finally {
          br.close();
        }

        docsSoFar++;
        if (docsSoFar >= maxToIndex) {
          break;
        }
        if (docsSoFar % increment == 0) {
          writer.close();
          writer = initWriter(docsSoFar);
        }
      }
    }

    // finish index
    int numIndexed = writer.docCount();
    writer.optimize();
    writer.close();
    
    return numIndexed;
  }

  // Print out stats for one run.
  private static void printInterimReport(int rep, float secs, 
                                         int numIndexed) {
    DecimalFormat secsFormat = new DecimalFormat("#,##0.00");
    String secString = secsFormat.format(secs);
    System.out.println(rep + "   Secs: " + secString + 
                       "  Docs: " + numIndexed);
  }

  // Print out aggregate stats
  private static void printFinalReport(float[] times) {
    // produce mean and truncated mean
    Arrays.sort(times);
    float meanTime = 0.0f;
    float truncatedMeanTime = 0.0f;
    int numToChop = times.length >> 2;
    int numKept = 0;
    for (int i = 0; i < times.length; i++) {
        meanTime += times[i];
        // discard fastest 25% and slowest 25% of reps
        if (i < numToChop || i >= (times.length - numToChop))
            continue;
        truncatedMeanTime += times[i];
        numKept++;
    }
    meanTime /= times.length;
    truncatedMeanTime /= numKept;
    int numDiscarded = times.length - numKept;
    DecimalFormat format = new DecimalFormat("#,##0.00");
    String meanString = format.format(meanTime);
    String truncatedMeanString = format.format(truncatedMeanTime);

    // get the Lucene version
    Package lucenePackage = org.apache.lucene.LucenePackage.get();
    String luceneVersion = lucenePackage.getSpecificationVersion();

    System.out.println("---------------------------------------------------");
    System.out.println("Lucene " +  luceneVersion);
    System.out.println("JVM " + System.getProperty("java.version") +
                       " (" + System.getProperty("java.vendor") + ")");
    System.out.println(System.getProperty("os.name") + " " + 
                       System.getProperty("os.version") + " " +
                       System.getProperty("os.arch"));
    System.out.println("Mean: " + meanString + " secs");
    System.out.println("Truncated mean (" +
                        numKept + " kept, " +
                        numDiscarded + " discarded): " + 
                        truncatedMeanString + " secs");
    System.out.println("---------------------------------------------------");
  }
}
