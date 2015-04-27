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

  testUnusableNameThrows: function(test) {
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

  testAlternateNameWorks: function(test) {
    test.expect(2);
    var Test = java.import("Test");
    test.ok(Test);
    Test.name_alt(function(err) {
      test.ifError(err);
      test.done();
    });
  }
}
