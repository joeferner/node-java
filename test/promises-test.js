var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");


exports['Promises'] = nodeunit.testCase({

  "create an instance of a class and call methods (getClassPromise & getNamePromise)": function(test) {
    // Adapted from a test in simple-test.js
    java.newInstance("java.util.ArrayList", function(err, list) {
      if (err) {
        console.log(err);
        return;
      }
      test.ok(list);
      if (list) {
        list.getClassPromise()
          .then(function(clazz) { return clazz.getNamePromise(); })
          .then(function(name) {
            test.equal(name, "java.util.ArrayList");
          })
          .catch(function(err) {
            test.ifError(err);
          })
          .then(function() {
            test.expect(2);
            test.done();
          });
      }
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
  }
});

