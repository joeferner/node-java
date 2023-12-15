// testDefault.js

// In the default case, the developer does not set asyncOptions.
// We should get the defacto standard behavior.

var java = require("../");
var nodeunit = require("nodeunit");

java.asyncOptions = undefined;

module.exports = {
  testAPI: function(test) {
    test.expect(5);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    test.ok(arrayList);
    test.ok(java.instanceOf(arrayList, "java.util.ArrayList"));

    test.ok(typeof arrayList.addSync !== 'undefined', 'Expected `addSync` to be present, but it is NOT.');
    test.ok(typeof arrayList.add !== 'undefined', 'Expected `add` to be present, but it is NOT.');
    test.ok(typeof arrayList.addPromise === 'undefined', 'Expected `addPromise` to NOT be present, but it is.');
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
    test.ok(!api.includes('formatAsync'), 'Expected `formatAsync` to NOT be present, but it is.');
    test.ok(!api.includes('formatPromise'), 'Expected `formatPromise` to NOT be present, but it is.');
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
  }
}
