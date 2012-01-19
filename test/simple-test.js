

var java = require("../");
var nodeunit = require("nodeunit");
var util = require("util");

exports['Simple'] = nodeunit.testCase({
  "create an instance of a class (async)": function(test) {
    java.newInstance("java.util.ArrayList", function(list) {
      test.ok(list);
      test.done();
    });
  }
});
