'use strict';

var bindings = require("../build/Release/nodejavabridge_bindings");

var java = module.exports = new bindings.Java();
java.classpath.push(__dirname + "/../commons-lang3-node-java.jar");

var MODIFIER_PUBLIC = 1;
var MODIFIER_STATIC = 8;

java.require = function (name) {
  var clazz = java.findClassSync(name); // TODO: change to Class.forName when classloader issue is resolved.
  var result = function () {
    var args = [name];
    for (var i = 0; i < arguments.length; i++) {
      args.push(arguments[i]);
    }
    return java.newInstanceSync.apply(java, args);
  };
  var i;

  // copy static fields
  var fields = clazz.getDeclaredFieldsSync();
  for (i = 0; i < fields.length; i++) {
    if (((fields[i].getModifiersSync() & MODIFIER_PUBLIC) === MODIFIER_PUBLIC)
      && ((fields[i].getModifiersSync() & MODIFIER_STATIC) === MODIFIER_STATIC)) {
      var fieldName = fields[i].getNameSync();
      result.__defineGetter__(fieldName, function (name, fieldName) {
        return java.getStaticFieldValue(name, fieldName);
      }.bind(this, name, fieldName));
      result.__defineSetter__(fieldName, function (name, fieldName, val) {
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

  return result;
};
