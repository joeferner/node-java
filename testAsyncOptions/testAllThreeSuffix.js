// testAllThreeSuffix.js

// All three variants have non-empty suffix, i.e a suffix is required for any variant.

var java = require("../");
var assert = require("assert");

java.asyncOptions = {
  syncSuffix: "Sync",
  asyncSuffix: "Async",
  promiseSuffix: 'Promise',
  promisify: require('when/node').lift         // https://github.com/cujojs/when
};

module.exports = {
  testAPI: function(test) {
    test.expect(6);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    test.ok(arrayList);
    test.ok(java.instanceOf(arrayList, "java.util.ArrayList"));

    test.ok(typeof arrayList.addSync !== 'undefined', 'Expected `addSync` to be present, but it is NOT.');
    test.ok(typeof arrayList.addAsync !== 'undefined', 'Expected `addAsync` to be present, but it is NOT.');
    test.ok(typeof arrayList.addPromise !== 'undefined', 'Expected `addPromise` to be present, but it is NOT.');
    test.ok(typeof arrayList.add === 'undefined', 'Expected `add` to NOT be present, but it is.');
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

    var api = Object.keys(String).filter((key) => typeof String[key] === 'function');
    test.ok(api.includes('formatSync'), 'Expected `formatSync` to be present, but it is NOT.');
    test.ok(api.includes('formatAsync'), 'Expected `formatAsync` to be present, but it is NOT.');
    test.ok(api.includes('formatPromise'), 'Expected `formatPromise` to be present, but it is NOT.');
    test.ok(!api.includes('format'), 'Expected `format` to NOT be present, but it is.');
    test.ok(!api.includes('formatundefined'), 'Expected `formatundefined` to NOT be present, but it is.');
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

  testStaticSyncCalls: function(test) {
    test.expect(1);
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    // Among other things, java.import creates Sync functions for static methods.
    var String = java.import("java.lang.String");
    test.strictEqual(String.formatSync('%s--%s', "hello", "world"), "hello--world");
    test.done();
  },

  testAsyncCalls: function(test) {
    test.expect(4);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.addAsync("hello", function(err, result) {
      test.ifError(err);
      arrayList.addAsync("world", function(err, result) {
        test.ifError(err);
        arrayList.sizeAsync(function(err, size) {
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
