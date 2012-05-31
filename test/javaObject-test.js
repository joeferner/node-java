
var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Java Object'] = nodeunit.testCase({
  setUp: function(callback) {
    this.testObj = java.newInstanceSync("Test");
    callback();
  },

  "field": function(test) {
    test.equal(this.testObj.nonstaticInt, 42);
    this.testObj.nonstaticInt = 112;
    test.equal(this.testObj.nonstaticInt, 112);
    test.done();
  }
});
