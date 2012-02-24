'use strict';

var java = require("../testHelpers").java;
var nodeunit = require("nodeunit");
var util = require("util");

exports['Import Class'] = nodeunit.testCase({
  tearDown: function (callback) {
    java.setStaticFieldValue("Test", "staticFieldInt", 42);
    callback();
  },

  "import": function (test) {
    var Test = java.import('Test');
    test.equals(42, Test.staticFieldInt);
    Test.staticFieldInt = 200;
    test.equals(200, Test.staticFieldInt);

    test.equals(100, Test.staticMethodSync(99));
    Test.staticMethod(99, function (err, result) {
      test.ok(!err);
      test.equals(100, result);

      var testObj = new Test(5);
      test.equals(5, testObj.getIntSync());
      test.done();
    });
  }
});
