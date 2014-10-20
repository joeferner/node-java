var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Java8'] = nodeunit.testCase({

  "create an instance of a class that uses lambda expressions": function(test) {
    java.newInstance("TestLambda", function(err, lambda) {
      if (err) {
        console.log(err);
        return;
      }
      test.ok(lambda);
      if (lambda) {
        lambda.getClass(function(err, result) {
          if (err) {
            console.log(err);
            return;
          }
          result.getName(function(err, result) {
            if (err) {
              console.log(err);
              return;
            }
            test.equal(result, "TestLambda");
            test.done();
          });
        });
      }
    });
  },

  "call methods of a class that uses lambda expressions": function(test) {
    var TestLambda = java.import('TestLambda');
    var lambda = new TestLambda();
    var sum = lambda.testLambdaAdditionSync(23, 42);
    test.equal(sum, 65);
    var diff = lambda.testLambdaSubtractionSync(23, 42);
    test.equal(diff, -19);
    test.done();
  },
});

