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
      if (err) {
        return test.done(err);
      }
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
    } catch (err) {
      test.ok(err.toString().match(/my exception/));
    }
    test.done();
  },

  "callMethodSync exception thrown from method": function(test) {
    var ex = java.newInstanceSync("java.lang.Exception", "my exception");
    var myTest = java.newInstanceSync("Test");
    try {
      myTest.methodThrowsSync(ex);
      test.fail("should throw");
    } catch (err) {
      test.ok(err.toString().match(/my exception/));
    }
    test.done();
  },

  "char array": function(test) {
    var charArray = java.newArray("char", "hello world\n".split(''));
    java.callStaticMethod("Test", "staticMethodCharArrayToString", charArray, function(err, result) {
      if (err) {
        return test.done(err);
      }
      test.ok(result);
      test.equal(result, "hello world\n");
      test.done();
    });
  },

  "java.lang.Long addition": function(test) {
    var javaLong = java.newInstanceSync("java.lang.Long", 5);
    test.equal(javaLong.toString(), '5');
    var result = javaLong + 1;
    test.equal(result, 6);
    test.done();
  },

  "java.lang.Long calls (java Long)": function(test) {
    var javaLong = java.newInstanceSync("java.lang.Long", 5);
    java.callStaticMethod("Test", "staticMethodLongToString", javaLong, function(err, result) {
      if (err) {
        return test.done(err);
      }
      test.ok(result);
      test.equal(result, "5");
      test.done();
    });
  },

  "Call method that returns a long": function(test) {
    java.callStaticMethod("Test", "staticMethodReturnLong", function(err, result) {
      if (err) {
        return test.done(err);
      }
      test.ok(result);
      test.equal(result.longValue, "9223372036854775807");
      test.done();
    });
  }
});
