// testSyncDefaultPlusPromise.js

// Just Sync and Promise, with Sync the default (i.e. no suffix).
// This is the configuration that RedSeal wants for use with Tinkerpop/Gremlin.

var java = require("../");
var assert = require("assert");

java.asyncOptions = {
  syncSuffix: "",
  promiseSuffix: 'P',
  promisify: require('when/node').lift         // https://github.com/cujojs/when
};

module.exports = {
  testAPI: function(test) {
    test.expect(6);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    test.ok(arrayList);
    test.ok(java.instanceOf(arrayList, "java.util.ArrayList"));

    test.ok(typeof arrayList.add !== 'undefined', 'Expected `add` to be present, but it is NOT.');
    test.ok(typeof arrayList.addP !== 'undefined', 'Expected `addP` to be present, but it is NOT.');
    test.ok(typeof arrayList.addSync === 'undefined', 'Expected `addSync` to NOT be present, but it is.');
    test.ok(typeof arrayList.addAsync === 'undefined', 'Expected `addAsync` to NOT be present, but it is.');
    test.done();
  },

  testImportClass: function(test) {
    test.expect(3);
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    // This test verifies the import runs without error.
    var ArrayList = java.import("java.util.ArrayList");
    test.ok(ArrayList);
    var arrayList = new ArrayList();
    test.ok(arrayList);
    test.strictEqual(arrayList.size(), 0);
    test.done();
  },

  testStaticAPI: function(test) {
    test.expect(6);
    var String = java.import("java.lang.String");
    test.ok(String);

    var api = Object.keys(String).filter((key) => typeof String[key] === 'function');
    test.ok(api.includes('format'), 'Expected `format` to be present, but it is NOT.');
    test.ok(api.includes('formatP'), 'Expected `formatP` to be present, but it is NOT.');
    test.ok(!api.includes('formatSync'), 'Expected `formatSync` to NOT be present, but it is.');
    test.ok(!api.includes('formatAsync'), 'Expected `formatAsync` to NOT be present, but it is.');
    test.ok(!api.includes('formatundefined'), 'Expected `formatundefined` to NOT be present, but it is.');
    test.done();
  },

  testSyncCalls: function(test) {
    test.expect(1);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.add("hello");
    arrayList.add("world");
    test.strictEqual(arrayList.size(), 2);
    test.done();
  },

  testStaticSyncCalls: function(test) {
    test.expect(1);
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    // Among other things, java.import creates Sync functions for static methods.
    var String = java.import("java.lang.String");
    test.strictEqual(String.format('%s--%s', "hello", "world"), "hello--world");
    test.done();
  },

  testPromiseCalls: function(test) {
    test.expect(1);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.addP("hello")
      .then(function () { return arrayList.addP("world"); })
      .then(function () { return arrayList.sizeP(); })
      .then(function (size) {
        test.strictEqual(size, 2);
        test.done();
      });
  }

}
