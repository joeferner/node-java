var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Java - Call Ambiguous Method'] = nodeunit.testCase({
  "staticMethodAmbiguous (sync) - int passed to double": function(test) {
    var result = java.callStaticMethodSync('Test', 'staticMethodAmbiguous(Ljava/lang/Double;)I', 1);
    test.equal(result, 1);
    test.done();
  },

  "staticMethodAmbiguous (sync) - double passed to int": function(test) {
    var result = java.callStaticMethodSync('Test', 'staticMethodAmbiguous(Ljava/lang/Integer;)I', 1.1);
    test.equal(result, 2);
    test.done();
  },

  "staticMethodAmbiguous (sync) - method not found wrong argument type": function(test) {
    try {
      java.callStaticMethodSync('Test', 'staticMethodAmbiguous(Ljava/lang/String;)I', 1);
      test.fail("should throw");
    } catch (e) {
      console.log(e);
    }
    test.done();
  },

  "staticMethodAmbiguous (sync) - method failed because argument count mismatch": function(test) {
    try {
      java.callStaticMethodSync('Test', 'staticMethodAmbiguous(Ljava/lang/String;)I', 1, 2);
      test.fail("should throw argument length mismatch");
    } catch (e) {
      console.log(e);
    }
    test.done();
  },

  "staticMethodAmbiguous - int passed to double": function(test) {
    java.callStaticMethod('Test', 'staticMethodAmbiguous(Ljava/lang/Double;)I', 1, function(err, result) {
      test.ok(!err);
      test.equal(result, 1);
      test.done();
    })
  },

  "staticMethodAmbiguous - double passed to int": function(test) {
    java.callStaticMethod('Test', 'staticMethodAmbiguous(Ljava/lang/Integer;)I', 1.1, function(err, result) {
      test.ok(!err);
      test.equal(result, 2);
      test.done();
    });
  },

  "staticMethodAmbiguous - method not found": function(test) {
    java.callStaticMethod('Test', 'staticMethodAmbiguous(Ljava/lang/String;)I', 1, function(err, result) {
      test.ok(err);
      console.log(err);
      test.done();
    });
  },

  "staticMethodAmbiguous - method argument count mismatch": function(test) {
    java.callStaticMethod('Test', 'staticMethodAmbiguous(Ljava/lang/String;)I', 1, 2, function(err, result) {
      test.ok(err);
      console.log(err);
      test.done();
    });
  },

  "methodAmbiguous (sync) - int passed to double": function(test) {
    var myTest = java.newInstanceSync("Test");
    var result = java.callMethodSync(myTest, 'staticMethodAmbiguous(Ljava/lang/Double;)I', 1);
    test.equal(result, 1);
    test.done();
  },

  "methodAmbiguous (sync) - double passed to int": function(test) {
    var myTest = java.newInstanceSync("Test");
    var result = java.callMethodSync(myTest, 'methodAmbiguous(Ljava/lang/Integer;)I', 1.1);
    test.equal(result, 2);
    test.done();
  },

  "methodAmbiguous (sync) - method not found wrong argument type": function(test) {
    var myTest = java.newInstanceSync("Test");
    try {
      java.callMethodSync(myTest, 'methodAmbiguous(Ljava/lang/String;)I', 1);
      test.fail("should throw");
    } catch (e) {
      console.log(e);
    }
    test.done();
  },

  "methodAmbiguous (sync) - method failed because argument count mismatch": function(test) {
    var myTest = java.newInstanceSync("Test");
    try {
      java.callMethodSync(myTest, 'methodAmbiguous(Ljava/lang/String;)I', 1, 2);
      test.fail("should throw argument length mismatch");
    } catch (e) {
      console.log(e);
    }
    test.done();
  }
});
