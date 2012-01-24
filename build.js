#!/usr/bin/env node

var path = require('path');
var util = require('util');
var fs = require('fs');
var childProcess = require('child_process');

function build(builder) {
  builder.appendUnique('CXXFLAGS', ['-Wall']);
  builder.appendUnique('CXXFLAGS', ['-Isrc/']);
  builder.appendUnique('CXXFLAGS', ['-g']);
  builder.appendUnique('CXXFLAGS', ['-D_FILE_OFFSET_BITS=64']);
  builder.appendUnique('CXXFLAGS', ['-D_LARGEFILE_SOURCE']);
  builder.appendUnique('CXXFLAGS', ['-DHAVE_CONFIG_H']);

  // MAC has a built in JVM
  if(path.existsSync("/System/Library/Frameworks/JavaVM.framework/")) {
    var jdkIncludeDir = process.env["JDK_INCLUDE_DIR"] || "/System/Library/Frameworks/JavaVM.framework/Headers";
    builder.appendUnique('CXXFLAGS', '-I' + jdkIncludeDir);
    builder.appendUnique('LINKFLAGS', '-framework JavaVM');
  } else {
    var javaHome = process.env["JAVA_HOME"];

    // JDK Include directory
    var jdkIncludeDir = process.env["JDK_INCLUDE_DIR"];
    if(!javaHome && !jdkIncludeDir) {
      builder.fail("You must set JAVA_HOME or JDK_INCLUDE_DIR environment variable");
    }
    jdkIncludeDir = jdkIncludeDir || path.join(javaHome, "include");
    builder.failIfNotExists(jdkIncludeDir, 'Could not find "%s" check JAVA_HOME or JDK_INCLUDE_DIR environment variable.');
    builder.appendUnique('CXXFLAGS', '-I' + jdkIncludeDir);

    // JDK additional include directory
    var jdkAdditionalIncludeDir = process.env["JDK_AUX_INCLUDE_DIR"] || path.join(jdkIncludeDir, "linux");
    builder.failIfNotExists(jdkAdditionalIncludeDir, 'Could not find "%s" check JAVA_HOME or JDK_AUX_INCLUDE_DIR environment variable.');
    builder.appendUnique('CXXFLAGS', '-I' + jdkAdditionalIncludeDir);

    // JDK lib directory
    var jdkLibDirGuess = null;
    if(javaHome) {
      if(path.existsSync(path.join(javaHome, "/jre/lib/i386/server/"))) {
        jdkLibDirGuess = path.join(javaHome, "/jre/lib/i386/server/");
      } else {
        jdkLibDirGuess = path.join(javaHome, "/jre/lib/amd64/server/");
      }
    }
    var jdkLibDir = process.env["JDK_LIB_DIR"];
    if(!jdkLibDirGuess && !jdkLibDir) {
      builder.fail("You must set JAVA_HOME or JDK_LIB_DIR environment variable");
    }
    jdkLibDir = jdkLibDir || jdkLibDirGuess;
    builder.failIfNotExists(jdkLibDir, 'Could not find "%s" check JAVA_HOME or JDK_LIB_DIR environment variable.');
    builder.appendUnique('LINKFLAGS', '-L' + jdkLibDir);
    builder.appendUnique('LINKFLAGS', '-Wl,-rpath,' + jdkLibDir);

    builder.appendUnique('LINKFLAGS', '-ljvm');
  }

  builder.target = "nodejavabridge_bindings";
  builder.appendSourceDir('./src');
  builder.appendUnique('CXXFLAGS', '-Isrc/');

  builder.compile();
  builder.link();
}

/* ----------------------------------------------------------------------------- */

function Builder() {
  this.flagGroups = {};
  this.target = "native_bindings";
  this.sourceFiles = [];
  this.verbose = true;
  this.nodeDir = this.getNodeDir();
  this.nodeIncludeDir = path.join(this.nodeDir, '..', 'include', 'node');

  this.appendUnique('CXXFLAGS', '-g');
  this.appendUnique('CXXFLAGS_NODE', '-D_LARGEFILE_SOURCE');
  this.appendUnique('CXXFLAGS_NODE', '-D_FILE_OFFSET_BITS=64');
  this.appendUnique('CXXFLAGS', '-I' + this.nodeIncludeDir);
}

Builder.prototype.getNodeDir = function() {
  return path.join(process.execPath, '..');
}

Builder.prototype.getFlags = function(flagGroupName) {
  var flags = this.flagGroups[flagGroupName];
  if(!flags) {
    flags = this.flagGroups[flagGroupName] = [];
  }
  return flags;
}

Builder.prototype.appendUnique = function(flagGroupName, newFlags) {
  if(!(newFlags instanceof Array)) newFlags = [ newFlags ];
  var flags = this.getFlags(flagGroupName);

  for(var i=0; i<newFlags.length; i++) {
    if(flags.indexOf(newFlags[i]) == -1) {
      flags.push(newFlags[i]);
    }
  }
}

Builder.prototype.appendSource = function(fileName) {
  this.sourceFiles.push(fileName);
}

Builder.prototype.appendSourceDir = function(dirName) {
  var files = fs.readdirSync(dirName);
  for(var i=0; i<files.length; i++) {
    var fileName = files[i];
    if(fileName.match(/\.cpp$/) || fileName.match(/\.c$/) || fileName.match(/\.cxx$/)) {
      this.appendSource(path.join(dirName, fileName));
    }
  }
}

Builder.prototype.getCompilerArgs = function(fileName) {
  var args = [];
  var flags = this.getFlags('CXXFLAGS');
  args = args.concat(flags);
  args.push(fileName);
  return args;
}

Builder.prototype.getCompiler = function() {
  return "g++";
}

Builder.prototype.compile = function() {
  if(!path.existsSync("build")) {
    fs.mkdirSync("build");
  }

  if(this.sourceFiles.length == 0) {
    this.fail("Nothing to compile!");
  }

  var compiler = this.getCompiler();

  for(var i=0; i<this.sourceFiles.length; i++) {
    var fileName = this.sourceFiles[i];
    console.log(util.format("[%d/%d] Compiling %s", i+1, this.sourceFiles.length, fileName));
    var args = this.getCompilerArgs(fileName);
    if(this.verbose) {
      console.log(compiler, args.join(' '));
    }
    var child = childProcess.spawn(compiler, args);
    child.stdout.on('data', function (data) {
      process.stdout.write(data);
    });
    child.stderr.on('data', function (data) {
      process.stderr.write(data);
    });
    child.on('exit', function(code) {
      console.log(code);
    });
  }

  console.log("Done compiling");
}

Builder.prototype.link = function() {

}

Builder.prototype.failIfNotExists = function(dirName, message) {
  if(!path.existsSync(dirName)) {
    message = message || "Could not find '%s'.";
    this.fail(message, dirName);
  }
}

Builder.prototype.fail = function(message) {
  var msg = util.format.apply(this, arguments);
  console.error("ERROR: " + msg);
  process.exit(1);
}

build(new Builder());

