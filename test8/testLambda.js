var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Java8'] = nodeunit.testCase({
  "call methods of a class that uses lambda expressions": function(test) {
    try {
      var TestLambda = java.import('TestLambda');
      var lambda = new TestLambda();
      var sum = lambda.testLambdaAdditionSync(23, 42);
      test.equal(sum, 65);
      var diff = lambda.testLambdaSubtractionSync(23, 42);
      test.equal(diff, -19);
    }
    catch (err) {
      var unsupportedVersion = java.instanceOf(err.cause, 'java.lang.UnsupportedClassVersionError');
      test.ok(unsupportedVersion);
      if (unsupportedVersion)
        console.log('JRE 1.8 not available');
      else
        console.error('Java8 test failed with unknown error:', err);
    }
    test.done();
  }
});

