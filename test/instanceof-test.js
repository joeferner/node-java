var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['instanceOf'] = nodeunit.testCase({
  "working": function(test) {
    var subclass = java.newInstanceSync("Test$SubClass");
    if (!java.instanceOf(subclass, "Test$SuperClass")) {
      test.fail(subclass.getNameSync() + " should be an instance of Test$SuperClass");
    }
    test.done();
  },

  "non-java object": function(test) {
    if (java.instanceOf({}, "Test$SuperClass")) {
      test.fail("javascript objects are not instances of anything");
    }
    test.done();
  },

  "bad type": function(test) {
    var subclass = java.newInstanceSync("Test$SubClass");
    try {
      java.instanceOf(subclass, "BadClassName");
      test.fail("should have thrown an exception.")
    } catch (e) {
      // OK
    }
    test.done();
  }
});
