// testDefactoPlusPromise.js

// The defacto case but with promises also enabled.

var java = require("../");
var assert = require("assert");
var _ = require('lodash');

java.asyncOptions = {
  syncSuffix: "Sync",
  asyncSuffix: "",
  promiseSuffix: 'Promise',
  promisify: require('when/node').lift         // https://github.com/cujojs/when
};

module.exports = {
  testAPI: function(test) {
    test.expect(5);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    test.ok(arrayList);
    test.ok(java.instanceOf(arrayList, "java.util.ArrayList"));

    var api = _.functions(arrayList);
    test.ok(_.includes(api, 'addSync'), 'Expected `addSync` to be present, but it is NOT.');
    test.ok(_.includes(api, 'add'), 'Expected `add` to be present, but it is NOT.');
    test.ok(_.includes(api, 'addPromise'), 'Expected `addPromise` to be present, but it is NOT.');
    test.done();
  },

  testSyncCalls: function(test) {
    test.expect(1);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.addSync("hello");
    arrayList.addSync("world");
    test.strictEqual(arrayList.sizeSync(), 2);
    test.done();
  },

  testAsyncCalls: function(test) {
    test.expect(4);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.add("hello", function(err, result) {
      test.ifError(err);
      arrayList.add("world", function(err, result) {
        test.ifError(err);
        arrayList.size(function(err, size) {
          test.ifError(err);
          test.strictEqual(size, 2);
          test.done();
        });
      });
    });
  },

  testPromiseCalls: function(test) {
    test.expect(1);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.addPromise("hello")
      .then(function () { return arrayList.addPromise("world"); })
      .then(function () { return arrayList.sizePromise(); })
      .then(function (size) {
        test.strictEqual(size, 2);
        test.done();
      });
  }
}
