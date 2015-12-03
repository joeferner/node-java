'use strict';

process.env.PATH += require('../build/jvm_dll_path.json');

var _ = require('lodash');
var async = require('async');
var path = require('path');
var fs = require('fs');
var binaryPath = null;
try {
  if(fs.statSync && fs.statSync(path.join(__dirname, "../build/Debug/nodejavabridge_bindings.node")).isFile()) {
    binaryPath = path.resolve(path.join(__dirname, "../build/Debug/nodejavabridge_bindings.node"));
    console.log('****** NODE-JAVA RUNNING IN DEBUG MODE ******');
  }
} catch(e) {
  // do nothing fs.statSync just couldn't find the file
}
if (!binaryPath) {
  binaryPath = path.resolve(path.join(__dirname, "../build/Release/nodejavabridge_bindings.node"));
}
var bindings = require(binaryPath);

var java = module.exports = new bindings.Java();
java.classpath.push(path.resolve(__dirname, "../commons-lang3-node-java.jar"));
java.classpath.push(path.resolve(__dirname, __dirname, "../src-java"));
java.classpath.pushDir = function(dir) {
  fs.readdirSync(dir).forEach(function(file) {
    java.classpath.push(path.resolve(dir, file));
  });
};
java.nativeBindingLocation = binaryPath;

var syncSuffix = undefined;
var asyncSuffix = undefined;
var ifReadOnlySuffix = '_';

var SyncCall = function(obj, method) {
  if (syncSuffix === undefined)
    throw new Error('Sync call made before jvm created');
  var syncMethodName = method + syncSuffix;
  if (syncMethodName in obj)
    return obj[syncMethodName].bind(obj);
  else
    throw new Error('Sync method not found:' + syncMethodName);
}

java.isJvmCreated = function() {
  return typeof java.onJvmCreated !== 'function';
}

var clients = [];

// We provide two methods for 'clients' of node-java to 'register' their use of java.
// By registering, a client gets the opportunity to be called asynchronously just before the JVM is created,
// and just after the JVM is created. The before hook function will typically be used to add to java.classpath.
// The function may peform asynchronous operations, such as async [glob](https://github.com/isaacs/node-glob)
// resolutions of wild-carded file system paths, and then notify when it has finished via either calling
// a node-style callback function, or by resolving a promise.

// A client can register function hooks to be called before and after the JVM is created.
// If the client doesn't need to be called back for either function, it can pass null or undefined.
// Both before and after here are assumed to be functions that accept one argument that is a node-callback function.
java.registerClient = function(before, after) {
  var before_, after_;
  if (java.isJvmCreated()) {
    throw new Error('java.registerClient() called after JVM already created.');
  }
  before_ = (before && before.length === 0) ?
    function(cb) { before(); cb(); } :
    before;

  after_ = (after && after.length === 0) ?
    function(cb) { after(); cb(); } :
    after;

  clients.push({before: before_, after: after_});
}

// A client can register function hooks to be called before and after the JVM is created.
// If the client doesn't need to be called back for either function, it can pass null or undefined.
// Both before and after here are assumed to be functions that return Promises/A+ `thenable` objects.
java.registerClientP = function(beforeP, afterP) {
  if (java.isJvmCreated()) {
    throw new Error('java.registerClient() called after JVM already created.');
  }
  clients.push({beforeP: beforeP, afterP: afterP});
}

function runBeforeHooks(done) {
  function iterator(client, cb) {
    try {
      if (client.before) {
        client.before(cb);
      }
      else if (client.beforeP) {
        client.beforeP().then(function(ignored) { cb(); }, function(err) { cb(err); });
      }
      else {
        cb();
      }
    }
    catch (err) {
      cb(err);
    }
  }
  async.each(clients, iterator, done);
}

function createJVMAsync(callback) {
  var ignore = java.newLong(0); // called just for the side effect that it will create the JVM
  callback();
}

function runAfterHooks(done) {
  function iterator(client, cb) {
    try {
      if (client.after) {
        client.after(cb);
      }
      else if (client.afterP) {
        client.afterP().then(function(ignored) { cb(); }, function(err) { cb(err); });
      }
      else {
        cb();
      }
    }
    catch (err) {
      cb(err);
    }
  }
  async.each(clients, iterator, done);
}

function initializeAll(done) {
  async.series([runBeforeHooks, createJVMAsync, runAfterHooks], done);
}

// This function ensures that the JVM has been launched, asynchronously. The application can be notified
// when the JVM is fully created via either a node callback function, or via a promise.
// If the parameter `callback` is provided, it is assume be a node callback function.
// If the parameter is not provided, and java.asyncOptions.promisify has been specified,
// then this function will return a promise, by promisifying itself and then calling that
// promisified function.
// This function may be called multiple times -- the 2nd and subsequent calls are no-ops.
// However, once this method has been called (or the JVM is launched as a side effect of calling other java
// methods), then clients can no longer use the registerClient API.
java.ensureJvm = function(callback) {

  // First see if the promise-style API should be used.
  // This must be done first in order to ensure the proper API is used.
  if (_.isUndefined(callback) && java.asyncOptions && _.isFunction(java.asyncOptions.promisify)) {
    // Create a promisified version of this function.
    var launchJvmPromise = java.asyncOptions.promisify(java.ensureJvm.bind(java));
    // Call the promisified function, returning its result, which should be a promise.
    return launchJvmPromise();
  }

  // If we get here, callback must be a node-style callback function. If not, throw an error.
  else if (!_.isFunction(callback)) {
    throw new Error('java.launchJvm(cb) requires its one argument to be a callback function.');
  }

  // Now check if the JVM has already been created. If so, we assume that the jvm was already successfully
  // launched, and we can just implement idempotent behavior, i.e. silently notify that the JVM has been created.
  else if (java.isJvmCreated()) {
    return setImmediate(callback);
  }

  // Finally, queue the initializeAll function.
  else {
    return setImmediate(initializeAll, callback);
  }
}

java.onJvmCreated = function() {
  if (java.asyncOptions) {
    syncSuffix = java.asyncOptions.syncSuffix;
    asyncSuffix = java.asyncOptions.asyncSuffix;
    if (typeof syncSuffix !== 'string') {
      throw new Error('In asyncOptions, syncSuffix must be defined and must a string');
    }
    var promiseSuffix = java.asyncOptions.promiseSuffix;
    var promisify = java.asyncOptions.promisify;
    if (typeof promiseSuffix === 'string' && typeof promisify === 'function') {
      var methods = ['newInstance', 'callMethod', 'callStaticMethod'];
      methods.forEach(function (name) {
        java[name + promiseSuffix] = promisify(java[name]);
      });
    } else if (typeof promiseSuffix === 'undefined' && typeof promisify === 'undefined') {
      // no promises
    } else {
      throw new Error('In asyncOptions, if either promiseSuffix or promisify is defined, both most be.');
    }

    if (_.isString(java.asyncOptions.ifReadOnlySuffix) && java.asyncOptions.ifReadOnlySuffix !== '') {
      ifReadOnlySuffix = java.asyncOptions.ifReadOnlySuffix;
    }
  } else {
    syncSuffix = 'Sync';
    asyncSuffix = '';
  }
}

var MODIFIER_PUBLIC = 1;
var MODIFIER_STATIC = 8;

function isWritable(prop) {
  // If the property has no descriptor, or wasn't explicitly marked as not writable or not configurable, assume it is.
  // We check both desc.writable and desc.configurable, since checking desc.writable alone is not sufficient
  // (e.g. for either .caller or .arguments).
  // It may be that checking desc.configurable is sufficient, but the specification doesn't make this definitive,
  // and there is no harm in checking both.
  if (prop === 'caller' || prop === 'arguments') { return false; }

  var desc = Object.getOwnPropertyDescriptor(function() {}, prop) || {};
  return desc.writable !== false &&  desc.configurable !== false;
}

function usableName(name) {
  if (!isWritable(name)) {
    name = name + ifReadOnlySuffix;
  }
  return name;
}

java.import = function(name) {
  var clazz = java.findClassSync(name); // TODO: change to Class.forName when classloader issue is resolved.
  var result = function javaClassConstructorProxy() {
    var args = [name];
    for (var i = 0; i < arguments.length; i++) {
      args.push(arguments[i]);
    }
    return java.newInstanceSync.apply(java, args);
  };
  var i;

  result.class = clazz;

  // copy static fields
  var fields = SyncCall(clazz, 'getDeclaredFields')();
  for (i = 0; i < fields.length; i++) {
    var modifiers = SyncCall(fields[i], 'getModifiers')();
    if (((modifiers & MODIFIER_PUBLIC) === MODIFIER_PUBLIC)
      && ((modifiers & MODIFIER_STATIC) === MODIFIER_STATIC)) {
      var fieldName = SyncCall(fields[i], 'getName')();
      var jsfieldName = usableName(fieldName);
      result.__defineGetter__(jsfieldName, function(name, fieldName) {
        return java.getStaticFieldValue(name, fieldName);
      }.bind(this, name, fieldName));
      result.__defineSetter__(jsfieldName, function(name, fieldName, val) {
        java.setStaticFieldValue(name, fieldName, val);
      }.bind(this, name, fieldName));
    }
  }

  var promisify = undefined;
  var promiseSuffix;
  if (java.asyncOptions && java.asyncOptions.promisify) {
    promisify = java.asyncOptions.promisify;
    promiseSuffix = java.asyncOptions.promiseSuffix;
  }

  // copy static methods
  var methods = SyncCall(clazz, 'getDeclaredMethods')();
  for (i = 0; i < methods.length; i++) {
    var modifiers = SyncCall(methods[i], 'getModifiers')();
    if (((modifiers & MODIFIER_PUBLIC) === MODIFIER_PUBLIC)
      && ((modifiers & MODIFIER_STATIC) === MODIFIER_STATIC)) {
      var methodName = SyncCall(methods[i], 'getName')();

      if (_.isString(syncSuffix)) {
        var syncName = usableName(methodName + syncSuffix);
        result[syncName] = java.callStaticMethodSync.bind(java, name, methodName);
      }

      if (_.isString(asyncSuffix)) {
        var asyncName = usableName(methodName + asyncSuffix);
        result[asyncName] = java.callStaticMethod.bind(java, name, methodName);
      }

      if (promisify && _.isString(promiseSuffix)) {
        var promiseName = usableName(methodName + promiseSuffix);
        result[promiseName] = promisify(java.callStaticMethod.bind(java, name, methodName));
      }
    }
  }

  // copy static classes/enums
  var classes = SyncCall(clazz, 'getDeclaredClasses')();
  for (i = 0; i < classes.length; i++) {
    var modifiers = SyncCall(classes[i], 'getModifiers')();
    if (((modifiers & MODIFIER_PUBLIC) === MODIFIER_PUBLIC)
      && ((modifiers & MODIFIER_STATIC) === MODIFIER_STATIC)) {
      var className = SyncCall(classes[i], 'getName')();
      var simpleName = SyncCall(classes[i], 'getSimpleName')();
      Object.defineProperty(result, simpleName, {
        get: function(result, simpleName, className) {
          var c = java.import(className);

          // memoize the import
          var d = Object.getOwnPropertyDescriptor(result, simpleName);
          d.get = function(c) { return c; }.bind(null, c);
          Object.defineProperty(result, simpleName, d);

          return c;
        }.bind(this, result, simpleName, className),
        enumerable: true,
        configurable: true
      });
    }
  }

  return result;
};
