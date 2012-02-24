
var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Java - Static Field'] = nodeunit.testCase({
  tearDown: function (callback) {
    java.setStaticFieldValue("Test", "staticFieldInt", 42);
    callback();
  },

  "getStaticFieldValue int": function(test) {
    var val = java.getStaticFieldValue("Test", "staticFieldInt");
    test.equal(val, 42);
    test.done();
  },

  "setStaticFieldValue int": function(test) {
    java.setStaticFieldValue("Test", "staticFieldInt", 112);
    var val = java.getStaticFieldValue("Test", "staticFieldInt");
    test.equal(val, 112);
    test.done();
  },

  "getStaticFieldValue double": function(test) {
    var val = java.getStaticFieldValue("Test", "staticFieldDouble");
    test.equal(val, 42.5);
    test.done();
  },

  "setStaticFieldValue double": function(test) {
    java.setStaticFieldValue("Test", "staticFieldDouble", 112.12);
    var val = java.getStaticFieldValue("Test", "staticFieldDouble");
    test.equal(val, 112.12);
    test.done();
  },

  "setStaticFieldValue double (set int)": function(test) {
    java.setStaticFieldValue("Test", "staticFieldDouble", 112);
    var val = java.getStaticFieldValue("Test", "staticFieldDouble");
    test.equal(val, 112);
    test.done();
  },
});
