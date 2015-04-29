// testUnusableMethodName.js

// For any function, the property 'name' is an unwritable property.
// The value returned by java.import(<classname>) is a constructor-like function that has the shape of the class.
// In particular, any static methods of the class will be added as properties of the function.
// If a class has a static method named 'name', then an exception woudld be thrown when
// node-java attempts to set assign the static method to the .name property of constructor-like function.
// As a workaround, node-java will append the `ifReadOnlySuffix` to the property name.

var _ = require('lodash');
var java = require("../");
var nodeunit = require("nodeunit");

module.exports = {

  launch: function(test) {
    test.expect(4);
    java.asyncOptions = {
      syncSuffix: "Sync",
      asyncSuffix: "",
      ifReadOnlySuffix: "_alt"
    };

    function before(callback) {
      java.classpath.push('test/');
      test.ok(!java.isJvmCreated());
      callback();
    }

    function after(callback) {
      test.ok(java.isJvmCreated());
      callback();
    }

    java.registerClient(before, after);

    java.ensureJvm(function(err) {
      test.ifError(err);
      test.ok(java.isJvmCreated());
      test.done();
    });
  },

  testUnusableMethodName_nameThrows: function(test) {
    test.expect(1);
    var Test = java.import("Test");
    test.ok(Test);
    test.throws(
      function() {
        Test.name(function(err) {
          test.fail();  // should not get here
        });
      },
      function(err) {
        if (err instanceof TypeError) {
          test.done();
          return true;
        } else {
          test.done(err);
          return false;
        }
      }
    );
  },

  testUnusableMethodName_callerThrows: function(test) {
    test.expect(1);
    var Test = java.import("Test");
    test.ok(Test);
    test.throws(
      function() {
        Test.caller(function(err) {
          test.fail();  // should not get here
        });
      },
      function(err) {
        if (err instanceof TypeError) {
          test.done();
          return true;
        } else {
          test.done(err);
          return false;
        }
      }
    );
  },

  testUnusableMethodName_argumentsThrows: function(test) {
    test.expect(1);
    var Test = java.import("Test");
    test.ok(Test);
    test.throws(
      function() {
        Test.arguments(function(err) {
          test.fail();  // should not get here
        });
      },
      function(err) {
        if (err instanceof TypeError) {
          test.done();
          return true;
        } else {
          test.done(err);
          return false;
        }
      }
    );
  },

  testAlternateMethodName_name_altWorks: function(test) {
    test.expect(3);
    var Test = java.import("Test");
    test.ok(Test);
    Test.name_alt(function(err, val) {
      test.ifError(err);
      test.strictEqual(val, "name");
      test.done();
    });
  },

  testAlternateMethodName_caller_altWorks: function(test) {
    test.expect(3);
    var Test = java.import("Test");
    test.ok(Test);
    Test.caller_alt(function(err, val) {
      test.ifError(err);
      test.strictEqual(val, "caller");
      test.done();
    });
  },

  testAlternateMethodName_arguments_altWorks: function(test) {
    test.expect(3);
    var Test = java.import("Test");
    test.ok(Test);
    Test.arguments_alt(function(err, val) {
      test.ifError(err);
      test.strictEqual(val, "arguments");
      test.done();
    });
  },

  testReservedFieldName: function(test) {
    test.expect(7);
    var TestEnum = java.import("Test$Enum");
    test.ok(TestEnum);

    // 'foo' and 'bar' are valid enum names
    test.strictEqual(TestEnum.foo.toStringSync(), "foo");
    test.strictEqual(TestEnum.bar.toStringSync(), "bar");

    // TestEnum.name is actually the name of the proxy constructor function.
    test.strictEqual(TestEnum.name, "javaClassConstructorProxy");

    // Instead we need to acccess TestEnum.name_alt
    test.strictEqual(TestEnum.name_alt.toString(), "name");
    test.strictEqual(TestEnum.caller_alt.toString(), "caller");
    test.strictEqual(TestEnum.arguments_alt.toString(), "arguments");

    test.done();
  },
}
