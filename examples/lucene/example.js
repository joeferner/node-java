#!/usr/bin/env node

var java = require("../../");
java.classpath.push("./lucene-lib/lucene-core-7.4.0.jar");
java.classpath.push("./lucene-lib/lucene-analyzers-common-7.4.0.jar");
java.classpath.push("./lucene-lib/lucene-queryparser-7.4.0.jar");


var idx = java.newInstanceSync("org.apache.lucene.store.RAMDirectory");
var analyzer = java.newInstanceSync("org.apache.lucene.analysis.standard.StandardAnalyzer");
var writerConfig = java.newInstanceSync("org.apache.lucene.index.IndexWriterConfig", analyzer);
var writer = java.newInstanceSync("org.apache.lucene.index.IndexWriter", idx, writerConfig);
var queryParser = java.newInstanceSync("org.apache.lucene.queryparser.classic.QueryParser", "content", analyzer);

writer.addDocumentSync(createDocument("Theodore Roosevelt",
  "It behooves every man to remember that the work of the " +
  "critic, is of altogether secondary importance, and that, " +
  "in the end, progress is accomplished by the man who does " +
  "things."));
writer.addDocumentSync(createDocument("Friedrich Hayek",
  "The case for individual freedom rests largely on the " +
  "recognition of the inevitable and universal ignorance " +
  "of all of us concerning a great many of the factors on " +
  "which the achievements of our ends and welfare depend."));
writer.addDocumentSync(createDocument("Ayn Rand",
  "There is nothing to take a man's freedom away from " +
  "him, save other men. To be free, a man must be free " +
  "of his brothers."));
writer.addDocumentSync(createDocument("Mohandas Gandhi",
  "Freedom is not worth having if it does not connote " +
  "freedom to err."));

writer.closeSync();

var searcher = java.newInstanceSync("org.apache.lucene.search.IndexSearcher", java.callStaticMethodSync("org.apache.lucene.index.DirectoryReader", "open", idx));

search(searcher, "freedom");
search(searcher, "free");
search(searcher, "progress or achievements");

function createDocument(title, content) {
  var fieldStoreYes = java.callStaticMethodSync("org.apache.lucene.document.Field$Store", "valueOf", "YES");
  var doc = java.newInstanceSync("org.apache.lucene.document.Document");
  doc.addSync(java.newInstanceSync("org.apache.lucene.document.TextField", "title", title, fieldStoreYes));
  doc.addSync(java.newInstanceSync("org.apache.lucene.document.TextField", "content", content, fieldStoreYes));
  return doc;
}

function search(searcher, queryString) {
  var query = queryParser.parseSync(queryString);
  var topDocs = searcher.searchSync(query, 10);

  console.log("Found " + topDocs.totalHits + " hits for query \"" + queryString + "\".");
  var scoreDocs = topDocs.scoreDocs;
  for(var i=0; i<topDocs.scoreDocs.length; i++) {
    var docId = scoreDocs[i].doc;
    var doc = searcher.docSync(docId);
    console.log("  " + (i + 1) + ". " + doc.getSync("title"));
  }
}
