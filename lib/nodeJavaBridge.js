'use strict';

process.env.PATH += require('../build/jvm_dll_path.json');

var path = require('path');
var binaryPath = path.resolve(path.join(__dirname, "../build/Release/nodejavabridge_bindings.node"));
var bindings = require(binaryPath);

var java = module.exports = new bindings.Java();
java.classpath.push(path.resolve(__dirname, "../commons-lang3-node-java.jar"));
java.classpath.push(path.resolve(__dirname, __dirname, "../src-java"));
java.nativeBindingLocation = binaryPath;

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
  var fields = clazz.getDeclaredFieldsSync();
  for (i = 0; i < fields.length; i++) {
    if (((fields[i].getModifiersSync() & MODIFIER_PUBLIC) === MODIFIER_PUBLIC)
      && ((fields[i].getModifiersSync() & MODIFIER_STATIC) === MODIFIER_STATIC)) {
      var fieldName = fields[i].getNameSync();
      result.__defineGetter__(fieldName, function(name, fieldName) {
        return java.getStaticFieldValue(name, fieldName);
      }.bind(this, name, fieldName));
      result.__defineSetter__(fieldName, function(name, fieldName, val) {
        java.setStaticFieldValue(name, fieldName, val);
      }.bind(this, name, fieldName));
    }
  }

  // copy static methods
  var methods = clazz.getDeclaredMethodsSync();
  for (i = 0; i < methods.length; i++) {
    if (((methods[i].getModifiersSync() & MODIFIER_PUBLIC) === MODIFIER_PUBLIC)
      && ((methods[i].getModifiersSync() & MODIFIER_STATIC) === MODIFIER_STATIC)) {
      var methodName = methods[i].getNameSync();
      result[methodName + 'Sync'] = java.callStaticMethodSync.bind(java, name, methodName);
      result[methodName] = java.callStaticMethod.bind(java, name, methodName);
    }
  }

  // copy static classes/enums
  var classes = clazz.getDeclaredClassesSync();
  for (i = 0; i < classes.length; i++) {
    if (((classes[i].getModifiersSync() & MODIFIER_PUBLIC) === MODIFIER_PUBLIC)
      && ((classes[i].getModifiersSync() & MODIFIER_STATIC) === MODIFIER_STATIC)) {
      var className = classes[i].getNameSync();
      var simpleName = classes[i].getSimpleNameSync();
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
