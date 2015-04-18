// testDefacto.js

// In the defacto case, the developer sets asyncOptions, but specifies the defacto standard behavior.

var _ = require('lodash');
var java = require("../");
var nodeunit = require("nodeunit");

module.exports = {

  launch: function(test) {
    test.expect(9);
    var api = _.functions(java);
    test.ok(_.includes(api, 'isJvmCreated'), 'Expected `isJvmCreated` to be present, but it is NOT.');
    test.ok(!java.isJvmCreated());

    java.asyncOptions = {
      syncSuffix: "Sync",
      asyncSuffix: ""
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
    java.registerClient(undefined, after);
    java.registerClient(before, undefined);

    java.ensureJvm(function(err) {
      test.ifError(err);
      test.ok(java.isJvmCreated());

      // Verify that ensureJvm is idempotent
      java.ensureJvm(function(err) {
        test.ifError(err);
        test.done();
      });
    });
  },

  testAPI: function(test) {
    test.expect(5);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    test.ok(arrayList);
    test.ok(java.instanceOf(arrayList, "java.util.ArrayList"));

    var api = _.functions(arrayList);
    test.ok(_.includes(api, 'addSync'), 'Expected `addSync` to be present, but it is NOT.');
    test.ok(_.includes(api, 'add'), 'Expected `add` to be present, but it is NOT.');
    test.ok(!_.includes(api, 'addPromise'), 'Expected addPromise to NOT be present, but it is.');
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
    test.ok(_.includes(api, 'format'), 'Expected `format` to be present, but it is NOT.');
    test.ok(_.includes(api, 'formatSync'), 'Expected `formatSync` to be present, but it is NOT.');
    test.ok(!_.includes(api, 'formatAsync'), 'Expected `formatAsync` to NOT be present, but it is.');
    test.ok(!_.includes(api, 'formatPromise'), 'Expected `formatPromise` to NOT be present, but it is.');
    test.ok(!_.includes(api, 'formatundefined'), 'Expected `formatundefined` to NOT be present, but it is.');
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
