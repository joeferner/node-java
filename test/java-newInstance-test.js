
var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Java - New Instance'] = nodeunit.testCase({
  "newInstance": function(test) {
    java.newInstance("Test", function(err, result) {
      test.ok(result);
      test.equal(result.getClassSync().toStringSync(), "class Test");
      test.ok(result.getInt);
      test.ok(result.getIntSync);
      test.ok(!result.staticMethod);
      test.ok(!result.staticMethodSync);
      test.equal(result.nonstaticInt, 42);
      test.done();
    });
  },

  "newInstanceSync": function(test) {
    var result = java.newInstanceSync("Test");
    test.ok(result);
    test.equal(result.getClassSync().toStringSync(), "class Test");
    test.done();
  },

  "newInstance with args": function(test) {
    java.newInstance("Test", 42, function(err, result) {
      test.ok(result);
      test.equal(result.getIntSync(), 42);
      test.done();
    });
  },

  "newInstanceSync with args": function(test) {
    var result = java.newInstanceSync("Test", 42);
    test.ok(result);
    test.equal(result.getIntSync(), 42);
    test.done();
  },

  "newInstance bad class name": function(test) {
    java.newInstance("BadClassName", function(err, result) {
      test.ok(err);
      test.ok(!result);
      test.done();
    });
  },

  "newInstanceSync bad class name": function(test) {
    test.throws(function() {
      java.newInstanceSync("BadClassName");
    });
    test.done();
  },

  "newInstance bad arg types": function(test) {
    java.newInstance("Test", "z", function(err, result) {
      test.ok(err);
      test.ok(!result);
      test.done();
    });
  },

  "newInstanceSync bad arg types": function(test) {
    test.throws(function() {
      java.newInstanceSync("Test", "z");
    });
    test.done();
  },

  "newInstance bad number of args": function(test) {
    java.newInstance("Test", 42, "z", function(err, result) {
      test.ok(err);
      test.ok(!result);
      test.done();
    });
  },

  "newInstanceSync bad number of args": function(test) {
    test.throws(function() {
      java.newInstanceSync("Test", 42, "z");
    });
    test.done();
  },

  "newInstance exception thrown from constructor": function(test) {
    var ex = java.newInstanceSync("java.lang.Exception", "my exception");
    java.newInstance("TestExceptions", ex, function(err, result) {
      test.ok(err);
      test.ok(err.toString().match(/my exception/));
      test.ok(!result);
      test.done();
    });
  },

  "newInstanceSync exception thrown from constructor": function(test) {
    var ex = java.newInstanceSync("java.lang.Exception", "my exception");
    try {
      java.newInstanceSync("TestExceptions", ex);
      test.fail("should throw");
    } catch(err) {
      test.ok(err.toString().match(/my exception/));
    }
    test.done();
  },
});
