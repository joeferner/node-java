#!/usr/bin/env node

var path = require('path');
var util = require('util');
var fs = require('fs');
var childProcess = require('child_process');

function build(builder) {
  builder.appendUnique('CXXFLAGS', ['-Wall']);
  builder.appendUnique('CXXFLAGS', ['-Isrc/']);
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

  builder.compileAndLink();
}

/* ----------------------------------------------------------------------------- */

function Builder() {
  this.flagGroups = {};
  this.target = "native_bindings";
  this.sourceFiles = [];
  this.verbose = false;
  this.cppCompiler = "g++";
  this.linker = "g++";
  this.objectFiles = [];

  for(var i=0; i<process.argv.length; i++) {
    var arg = process.argv[i];
    if(arg == '-v' || arg == '--verbose') {
      this.verbose = true;
    }
  }

  this.nodeDir = this.getNodeDir();
  this.nodeIncludeDir = path.join(this.nodeDir, '..', 'include', 'node');
  this.nodeLibDir = path.join(this.nodeDir, '..', 'lib');
  this.projectDir = path.resolve('.');
  this.buildDir = path.resolve(this.projectDir, 'build');
  this.ouputDir = path.resolve(this.buildDir, 'Release');
  
  this.appendUnique('CXXFLAGS', [
    '-g',
    '-c',
    '-fPIC',
    '-DPIC',
    '-D_LARGEFILE_SOURCE',
    '-D_FILE_OFFSET_BITS=64',
    '-D_GNU_SOURCE',
    '-I' + this.nodeIncludeDir
  ]);
  
  this.appendUnique('LINKFLAGS', [
    '-shared',
    '-L' + this.nodeLibDir
  ]);
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

Builder.prototype.getCompilerArgs = function(fileName, outFileName) {
  fileName = path.resolve(fileName);
  this.createDir(path.dirname(outFileName));
  var args = [];
  var flags = this.getFlags('CXXFLAGS');
  args = args.concat(flags);
  args.push(fileName);
  args.push("-o");
  args.push(outFileName);
  return args;
}

Builder.prototype.getLinkerArgs = function(outFileName) {
  this.createDir(path.dirname(outFileName));
  var args = [];
  var flags = this.getFlags('LINKFLAGS');
  args = args.concat(this.objectFiles);
  args.push("-o");
  args.push(outFileName);
  args = args.concat(flags);
  return args;
}

Builder.prototype.createDir = function(dirName) {
  var parent = path.dirname(dirName);
  if(!path.existsSync(parent)) {
    this.createDir(parent);
  }
  if(!path.existsSync(dirName)) {
    fs.mkdirSync(dirName);
  }
}

Builder.prototype.run = function(cmd, args, callback) {
  var child = childProcess.spawn(cmd, args);
  child.stdout.on('data', function (data) {
    process.stdout.write(data);
  });
  child.stderr.on('data', function (data) {
    process.stderr.write(data);
  });
  child.on('exit', function(code) {
    callback(code);
  });
}

Builder.prototype._compile = function(curFileIdx, callback) {
  var self = this;
  var fileName = path.resolve(this.sourceFiles[curFileIdx]);
  var outFileName = path.join(this.ouputDir, path.relative(this.projectDir, fileName));
  outFileName = outFileName.replace(/\.cpp$/, '.o');
  this.objectFiles.push(outFileName);
  
  console.log(util.format(
    "[%d/%d] cxx: %s -> %s",
    this.currentTask+1,
    this.totalTasks,
    path.relative(this.projectDir, fileName),
    path.relative(this.projectDir, outFileName)));
  var args = this.getCompilerArgs(fileName, outFileName);
  if(this.verbose) {
    console.log(this.cppCompiler, args.join(' '));
  }
  this.run(this.cppCompiler, args, function(code) {
    self.currentTask++;
    callback(code);
  });
}

Builder.prototype.compile = function(callback) {
  var self = this;
  this.createDir(this.ouputDir);

  if(this.sourceFiles.length == 0) {
    callback(new Error("Nothing to compile!"));
    return;
  }

  var curFileIdx = 0;
  var doCompile;
  var err = false;
  doCompile = function() {
    if(curFileIdx < self.sourceFiles.length) {
      self._compile(curFileIdx, function(code) {
        if(code != 0) {
          err = true;
        }
        curFileIdx++;
        doCompile();
      });
    } else {
      if(self.verbose) {
        console.log("Done compiling.");
      }
      if(err) {
        callback(new Error("At least one file failed to compile."));
      } else {
        callback();
      }
    }
  }
  doCompile();
}

Builder.prototype.link = function(callback) {
  var self = this;
  this.createDir(this.ouputDir);  
  
  var outFileName = path.resolve(path.join(this.ouputDir, this.target + ".node"));
  console.log(util.format(
    "[%d/%d] cxx_link: %s -> %s",
    this.currentTask+1,
    this.totalTasks,
    this.objectFiles.map(function(f) { return path.relative(self.projectDir, f); }).join(' '),
    path.relative(this.projectDir, outFileName)));
  
  var args = this.getLinkerArgs(outFileName);

  if(this.verbose) {
    console.log(this.linker, args.join(' '));
  }

  this.run(this.linker, args, function(code) {
    self.currentTask++;
    if(self.verbose) {
      console.log("Done linking.");
    }
    if(code != 0) {
      callback(new Error("Failed to link."));
    } else {
      callback();
    }
  });
}

Builder.prototype.compileAndLink = function(callback) {
  this.currentTask = 0;
  this.totalTasks = this.sourceFiles.length + 1; // +1 is for linking
  callback = callback || function() {};
  var self = this;
  this.compile(function(err) {
    if(err) { self.fail(err); return; }
    self.link(function(err) {
      if(err) { self.fail(err); return; }
      if(self.verbose) {
        console.log("Done.");
      }
      callback();
    });
  });
}

Builder.prototype.failIfNotExists = function(dirName, message) {
  if(!path.existsSync(dirName)) {
    message = message || "Could not find '%s'.";
    this.fail(message, dirName);
  }
}

Builder.prototype.fail = function(message) {
  if(message instanceof Error) {
    message = message.message;
  }
  var msg = util.format.apply(this, arguments);
  console.error("ERROR: " + msg);
  process.exit(1);
}

build(new Builder());

