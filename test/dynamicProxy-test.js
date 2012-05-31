'use strict';

var java = require("../testHelpers").java;
var nodeunit = require("nodeunit");
var util = require("util");

exports['Dynamic Proxy'] = nodeunit.testCase({
  "0 Arguments": function (test) {
    var callCount = 0;

    var myProxy = java.newDynamicProxy('RunInterface$Interface0Arg', {
      run: function () {
        callCount++;
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    runInterface.run0ArgsSync(myProxy);

    test.equals(callCount, 2);

    test.done();
  },

  "1 Arguments": function (test) {
    var runData = '';

    var myProxy = java.newDynamicProxy('RunInterface$Interface1Arg', {
      run: function (str) {
        runData += str;
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    runInterface.run1ArgsSync(myProxy);

    test.equals(runData, 'test1test1');

    test.done();
  },

  "1 Arguments with return data": function (test) {
    var myProxy = java.newDynamicProxy('RunInterface$InterfaceWithReturn', {
      run: function (i) {
        return i + 1;
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    var result = runInterface.runWithReturnSync(myProxy);

    test.equals(result, 43);

    test.done();
  }
});
