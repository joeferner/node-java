
var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Java - Call Static Method'] = nodeunit.testCase({
  "callStaticMethod": function(test) {
    java.callStaticMethod("Test", "staticMethod", function(err, result) {
      test.ok(result);
      test.equal(result, "staticMethod called");
      test.done();
    });
  },

  "callStaticMethodSync": function(test) {
    var result = java.callStaticMethodSync("Test", "staticMethod");
    test.ok(result);
    test.equal(result, "staticMethod called");
    test.done();
  },

  "callStaticMethod with args": function(test) {
    java.callStaticMethod("Test", "staticMethod", 42, function(err, result) {
      test.ok(result);
      test.equal(result, 43);
      test.done();
    });
  },

  "callStaticMethodSync with args": function(test) {
    var result = java.callStaticMethodSync("Test", "staticMethod", 42);
    test.ok(result);
    test.equal(result, 43);
    test.done();
  },

  "callStaticMethod bad class name": function(test) {
    java.callStaticMethod("BadClassName", "staticMethod", function(err, result) {
      test.ok(err);
      test.ok(!result);
      test.done();
    });
  },


  "callStaticMethodSync bad class name": function(test) {
    test.throws(function() {
      java.callStaticMethodSync("BadClassName", "staticMethod");
    });
    test.done();
  },

  "callStaticMethod bad arg types": function(test) {
    java.callStaticMethod("Test", "staticMethod", "z", function(err, result) {
      test.ok(err);
      test.ok(!result);
      test.done();
    });
  },

  "callStaticMethodSync bad arg types": function(test) {
    test.throws(function() {
      java.callStaticMethodSync("Test", "staticMethod", "z");
    });
    test.done();
  },

  "callStaticMethod bad number of args": function(test) {
    java.callStaticMethod("Test", "staticMethod", 42, "z", function(err, result) {
      test.ok(err);
      test.ok(!result);
      test.done();
    });
  },

  "callStaticMethodSync bad number of args": function(test) {
    test.throws(function() {
      java.callStaticMethodSync("Test", "staticMethod", 42, "z");
    });
    test.done();
  },

  "callStaticMethod bad method name": function(test) {
    java.callStaticMethod("Test", "badMethodName", function(err, result) {
      test.ok(err);
      test.ok(!result);
      test.done();
    });
  },

  "callStaticMethodSync bad method name": function(test) {
    test.throws(function() {
      java.callStaticMethodSync("Test", "badMethodName");
    });
    test.done();
  },

  "callStaticMethod exception thrown from method": function(test) {
    var ex = java.newInstanceSync("java.lang.Exception", "my exception");
    java.callStaticMethod("Test", "staticMethodThrows", ex, function(err, result) {
      test.ok(err);
      test.ok(err.toString().match(/my exception/));
      test.ok(!result);
      test.done();
    });
  },

  "callStaticMethodSync exception thrown from method": function(test) {
    var ex = java.newInstanceSync("java.lang.Exception", "my exception");
    try {
      java.callStaticMethodSync("Test", "staticMethodThrows", ex);
      test.fail("should throw");
    } catch(err) {
      test.ok(err.toString().match(/my exception/));
    }
    test.done();
  },

  "callStaticMethodSync exception thrown from method (new exception)": function(test) {
    try {
      java.callStaticMethodSync("Test", "staticMethodThrowsNew");
      test.fail("should throw");
    } catch(err) {
      test.ok(err.toString().match(/my exception/));
    }
    test.done();
  }
});
