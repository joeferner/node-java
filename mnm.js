#!/usr/bin/env node

var path = require('path');
var fs = require('fs');
var Builder = require('mnm');
var builder = new Builder();

var existsSync = fs.existsSync || path.existsSync;
var javaHome = builder.trimQuotes(process.env["JAVA_HOME"]);

builder.appendUnique('CXXFLAGS', ['-Isrc/']);
builder.appendUnique('CXXFLAGS', ['-DHAVE_CONFIG_H']);

// MAC has a built in JVM
if (existsSync("/System/Library/Frameworks/JavaVM.framework/")) {
  var jdkIncludeDir = "/System/Library/Frameworks/JavaVM.framework/Headers";
  if(process.env["JDK_INCLUDE_DIR"]) jdkIncludeDir = process.env["JDK_INCLUDE_DIR"];
  else if(javaHome && javaHome.substr(0, 7) !== '/System') jdkIncludeDir = path.join(javaHome, 'include');
  
  builder.appendUnique('CXXFLAGS', '-I' + jdkIncludeDir);
  if(jdkIncludeDir.substr(0, 7) !== '/System') builder.appendUnique('CXXFLAGS', '-I' + path.join(jdkIncludeDir, 'darwin'));
  builder.appendUnique('LINKFLAGS', ['-framework', 'JavaVM']);
} else {
  // JDK Include directory
  var jdkIncludeDir = process.env["JDK_INCLUDE_DIR"];
  if (!javaHome && !jdkIncludeDir) {
    builder.fail("You must set JAVA_HOME or JDK_INCLUDE_DIR environment variable");
  }
  jdkIncludeDir = jdkIncludeDir || path.join(javaHome, "include");
  builder.failIfNotExists(jdkIncludeDir, 'Could not find "%s" check JAVA_HOME or JDK_INCLUDE_DIR environment variable.');
  builder.appendUnique('CXXFLAGS', '-I' + jdkIncludeDir);

  // JDK additional include directory
  var jdkAdditionalIncludeDirGuess;
  if (process.platform == 'win32') {
    jdkAdditionalIncludeDirGuess = path.join(jdkIncludeDir, "win32");
  } else {
    jdkAdditionalIncludeDirGuess = path.join(jdkIncludeDir, "linux");
  }
  var jdkAdditionalIncludeDir = process.env["JDK_AUX_INCLUDE_DIR"] || jdkAdditionalIncludeDirGuess;
  builder.failIfNotExists(jdkAdditionalIncludeDir, 'Could not find "%s" check JAVA_HOME or JDK_AUX_INCLUDE_DIR environment variable.');
  builder.appendUnique('CXXFLAGS', '-I' + jdkAdditionalIncludeDir);

  // JDK lib directory
  if (process.platform == 'win32') {
    var jdkLibDir = process.env["JDK_LIB_DIR"] || path.join(javaHome, "lib");
    if (!jdkLibDir) {
      builder.fail("You must set JAVA_HOME or JDK_LIB_DIR environment variable");
    }
    builder.appendLinkerSearchDir(jdkLibDir);
  } else {
    var jdkLibDirGuess = null;
    if (javaHome) {
      if (existsSync(path.join(javaHome, "/jre/lib/i386/server/"))) {
        jdkLibDirGuess = path.join(javaHome, "/jre/lib/i386/server/");
      } else {
        jdkLibDirGuess = path.join(javaHome, "/jre/lib/amd64/server/");
      }
    }
    var jdkLibDir = process.env["JDK_LIB_DIR"];
    if (!jdkLibDirGuess && !jdkLibDir) {
      builder.fail("You must set JAVA_HOME or JDK_LIB_DIR environment variable");
    }
    jdkLibDir = jdkLibDir || jdkLibDirGuess;
    builder.failIfNotExists(jdkLibDir, 'Could not find "%s" check JAVA_HOME or JDK_LIB_DIR environment variable.');
    builder.appendLinkerSearchDir(jdkLibDir);
    builder.appendUnique('LINKFLAGS', '-Wl,-rpath,' + jdkLibDir);
  }

  builder.appendLinkerLibrary('jvm');
}

builder.target = "nodejavabridge_bindings";
builder.appendSourceDir('./src');
builder.appendUnique('CXXFLAGS', '-Isrc/');

builder.run();
