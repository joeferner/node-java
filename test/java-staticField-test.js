
var java = require("./testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Java - Static Field'] = nodeunit.testCase({
  "getStaticFieldValue int": function(test) {
    var val = java.getStaticFieldValue("Test", "staticFieldInt");
    test.equal(val, 42);
    test.done();
  },

  "setStaticFieldValue int": function(test) {
    java.setStaticFieldValue("Test", "staticFieldInt", 112);
    var val = java.getStaticFieldValue("Test", "staticFieldInt");
    test.equal(val, 112);
    test.done();
  },
});
