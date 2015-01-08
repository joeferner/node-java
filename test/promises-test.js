var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Promises'] = nodeunit.testCase({
  "create an instance of a class and call methods (getClassPromise & getNamePromise)": function(test) {
    // Adapted from a test in simple-test.js
    java.newInstance("java.util.ArrayList", function(err, list) {
      test.ifError(err);
      test.ok(list);
      list.getClassPromise()
        .then(function(clazz) {
          test.ok(clazz);
          return clazz.getNamePromise();
        })
        .then(function(name) {
          test.equal(name, "java.util.ArrayList");
        })
        .catch(function(err) {
          test.ifError(err);
        })
        .then(function() {
          test.expect(4);
          test.done();
        });
    });
  },

  "import and execute promisified static method": function (test) {
    var Test = java.import('Test');
    Test.staticMethodPromise(99)
      .then(function (result) {
        test.equals(100, result);
      })
      .catch(function (err) {
        test.ifError(err);
      })
      .then(function() {
        test.expect(1);
        test.done();
      });
  },

  "run promisified method of Java module (newInstancePromise)": function (test) {
    java.newInstancePromise("java.util.ArrayList")
      .then(function(list) {
        test.ok(list);
        return list.getClassPromise();
      })
      .then(function(clazz) {
        test.ok(clazz);
        return clazz.getNamePromise();
      })
      .then(function(name) {
        test.equal(name, "java.util.ArrayList");
      })
      .catch(function(err) {
        test.ifError(err);
      })
      .then(function() {
        test.expect(3);
        test.done();
      });
  }
});

