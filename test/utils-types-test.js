
var java = require("./testHelpers").java;

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
  }
});
