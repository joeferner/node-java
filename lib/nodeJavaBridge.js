'use strict';

process.env.PATH += require('../build/jvm_dll_path.json');

var path = require('path');
var binaryPath = path.resolve(path.join(__dirname, "../build/Release/nodejavabridge_bindings.node"));
var bindings = require(binaryPath);

var java = module.exports = new bindings.Java();
java.classpath.push(path.resolve(__dirname, "../commons-lang3-node-java.jar"));
java.classpath.push(path.resolve(__dirname, __dirname, "../src-java"));
java.nativeBindingLocation = binaryPath;

var syncSuffix = undefined;
var asyncSuffix = undefined;

var SyncCall = function(obj, method) {
  if (syncSuffix === undefined)
    throw new Error('Sync call made before jvm created');
  var syncMethodName = method + syncSuffix;
  if (syncMethodName in obj)
    return obj[syncMethodName].bind(obj);
  else
    throw new Error('Sync method not found:' + syncMethodName);
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
  } else {
    syncSuffix = 'Sync';
    asyncSuffix = '';
  }
}

var MODIFIER_PUBLIC = 1;
var MODIFIER_STATIC = 8;


java.import = function(name) {
  var clazz = java.findClassSync(name); // TODO: change to Class.forName when classloader issue is resolved.
  var result = function() {
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
      result.__defineGetter__(fieldName, function(name, fieldName) {
        return java.getStaticFieldValue(name, fieldName);
      }.bind(this, name, fieldName));
      result.__defineSetter__(fieldName, function(name, fieldName, val) {
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
      result[methodName + syncSuffix] = java.callStaticMethodSync.bind(java, name, methodName);
      if (typeof asyncSuffix === 'string') {
        result[methodName + asyncSuffix] = java.callStaticMethod.bind(java, name, methodName);
      }
      if (promisify) {
        result[methodName + promiseSuffix] = promisify(java.callStaticMethod.bind(java, name, methodName));
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
