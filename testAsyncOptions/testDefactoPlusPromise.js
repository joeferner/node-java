// testDefactoPlusPromise.js

// The defacto case but with promises also enabled.

var java = require("../");
var assert = require("assert");

module.exports = {
  launch: function(test) {
    test.expect(7);
    var api = Object.keys(java).filter((key) => typeof java[key] === 'function');
    test.ok(api.includes('isJvmCreated'), 'Expected `isJvmCreated` to be present, but it is NOT.');
    test.ok(!java.isJvmCreated());

    java.asyncOptions = {
      syncSuffix: "Sync",
      asyncSuffix: "",
      promiseSuffix: 'Promise',
      promisify: require('when/node').lift         // https://github.com/cujojs/when
    };

    function before(callback) {
      test.ok(!java.isJvmCreated());
      callback();
    }

    function after(callback) {
      test.ok(java.isJvmCreated());
      callback();
    }

    java.registerClient(before, after);
    java.registerClient(null, after);
    java.registerClient(before);

    java.ensureJvm().done(function() {
      test.ok(java.isJvmCreated());
      test.done();
    });
  },

  testAPI: function(test) {
    test.expect(5);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    test.ok(arrayList);
    test.ok(java.instanceOf(arrayList, "java.util.ArrayList"));

    test.ok(arrayList.addSync !== 'undefined', 'Expected `addSync` to be present, but it is NOT.');
    test.ok(arrayList.add !== 'undefined', 'Expected `add` to be present, but it is NOT.');
    test.ok(arrayList.addPromise !== 'undefined', 'Expected `addPromise` to be present, but it is NOT.');
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
    test.ok(api.includes('format'), 'Expected `format` to be present, but it is NOT.');
    test.ok(api.includes('formatSync'), 'Expected `formatSync` to be present, but it is NOT.');
    test.ok(api.includes('formatPromise'), 'Expected `formatPromise` to be present, but it is NOT.');
    test.ok(!api.includes('formatAsync'), 'Expected `formatAsync` to NOT be present, but it is.');
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
