// testNoAsync.js

// Just Sync and Promise, both with a non-empty suffix.

var java = require("../");
var assert = require("assert");
var _ = require('lodash');

java.asyncOptions = {
  syncSuffix: "Sync",
  promiseSuffix: 'Promise',
  promisify: require('when/node').lift         // https://github.com/cujojs/when
};

module.exports = {
  testAPI: function(test) {
    test.expect(6);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    test.ok(arrayList);
    test.ok(java.instanceOf(arrayList, "java.util.ArrayList"));

    var api = _.functions(arrayList);
    test.ok(_.includes(api, 'addSync'), 'Expected `addSync` to be present, but it is NOT.');
    test.ok(_.includes(api, 'addPromise'), 'Expected `addPromise` to be present, but it is NOT.');
    test.ok(!_.includes(api, 'add'), 'Expected `add` to NOT be present, but it is.');
    test.ok(!_.includes(api, 'addAsync'), 'Expected `addAsync` to NOT be present, but it is.');
    test.done();
  },

  testImportClass: function(test) {
    test.expect(3);
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    var ArrayList = java.import("java.util.ArrayList");
    test.ok(ArrayList);
    var arrayList = new ArrayList();
    test.ok(arrayList);
    test.strictEqual(arrayList.sizeSync(), 0);
    test.done();
  },

  testStaticAPI: function(test) {
    test.expect(6);
    var String = java.import("java.lang.String");
    test.ok(String);

    var api = _.functions(String);
    test.ok(_.includes(api, 'joinSync'), 'Expected `joinSync` to be present, but it is NOT.');
    test.ok(_.includes(api, 'joinPromise'), 'Expected `joinPromise` to be present, but it is NOT.');
    test.ok(!_.includes(api, 'join'), 'Expected `join` to NOT be present, but it is.');
    test.ok(!_.includes(api, 'joinAsync'), 'Expected `joinAsync` to NOT be present, but it is.');
    test.ok(!_.includes(api, 'joinundefined'), 'Expected `joinundefined` to NOT be present, but it is.');
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
