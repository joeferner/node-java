
var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Utils - Types'] = nodeunit.testCase({
  "Array of Objects": function(test) {
    var val = java.getStaticFieldValue("Test", "staticArrayObjects");
    test.equal(null, val);

    java.setStaticFieldValue("Test", "staticArrayObjects", java.newArray("Test", [
      java.newInstanceSync("Test", 1),
      java.newInstanceSync("Test", 2),
      java.newInstanceSync("Test", 3)
    ]));

    val = java.getStaticFieldValue("Test", "staticArrayObjects");
    test.ok(val);
    test.equal(val.length, 3);
    test.equal(val[0].getIntSync(), 1);
    test.equal(val[1].getIntSync(), 2);
    test.equal(val[2].getIntSync(), 3);
    test.done();
  },

  "Static Method Overload": function(test) {
    var result = java.callStaticMethodSync("Test", "staticMethodOverload", "a");
    test.equal(result, 1);
    result = java.callStaticMethodSync("Test", "staticMethodOverload", 1);
    test.equal(result, 2);
    result = java.callStaticMethodSync("Test", "staticMethodOverload", java.newInstanceSync("Test$SuperClass"));
    test.equal(result, 3);
    result = java.callStaticMethodSync("Test", "staticMethodOverload", java.newInstanceSync("Test$SubClass"));
    test.equal(result, 4);
    test.done();
  },

  "Method Overload": function(test) {
    var testObj = java.newInstanceSync("Test");
    var result = testObj.methodOverloadSync("a");
    test.equal(result, 1);
    result = testObj.methodOverloadSync(1);
    test.equal(result, 2);
    result = testObj.methodOverloadSync(java.newInstanceSync("Test$SuperClass"));
    test.equal(result, 3);
    result = testObj.methodOverloadSync(java.newInstanceSync("Test$SubClass"));
    test.equal(result, 4);
    test.done();
  },

  "Char array": function(test) {
    var originalArray = "hello 世界\n".split('');
    var Arrays = java.import("java.util.Arrays");
    var arr1 = java.newArray("char", originalArray);
    var list = Arrays.asListSync(arr1);
    var arr2 = list.toArraySync();
    test.equal(arr2.length, 1);
    test.equal(arr2[0].length, 9);
    var isTypedArrayReturn = !(typeof arr2[0][0] === 'string');
    for(var i=0; i<originalArray.length; i++) {
      if (isTypedArrayReturn) {
        test.equal(arr2[0][i], originalArray[i].charCodeAt(0));
      } else {
        test.equal(arr2[0][i], originalArray[i]);
      }
    }
    test.done();
  }
});
