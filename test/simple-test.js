

var java = require("../");
var nodeunit = require("nodeunit");
var util = require("util");

exports['Simple'] = nodeunit.testCase({
  "create an instance of a class (async)": function(test) {
    java.newInstance("java.util.ArrayList", function(err, list) {
      if(err) { console.log(err); return; }
      console.log(list);
      test.ok(list);
      test.done();
    });
  }
});
