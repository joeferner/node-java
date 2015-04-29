// testAsyncSuffixSyncDefault.js

// Use "Async" for the asyncSuffix, and "" for the syncSuffix.

var java = require("../");
var assert = require("assert");
var _ = require('lodash');

module.exports = {
  launch: function(test) {
    test.expect(4);
    java.asyncOptions = {
      syncSuffix: "",
      asyncSuffix: "Async",
      ifReadOnlySuffix: "_alt"
    };

    function before(callback) {
      java.classpath.push('test/');
      test.ok(!java.isJvmCreated());
      callback();
    }

    function after(callback) {
      test.ok(java.isJvmCreated());
      callback();
    }

    java.registerClient(before, after);

    java.ensureJvm(function(err) {
      test.ifError(err);
      test.ok(java.isJvmCreated());
      test.done();
    });
  },

  testAPI: function(test) {
    test.expect(5);
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    test.ok(arrayList);
    test.ok(java.instanceOf(arrayList, "java.util.ArrayList"));

    var api = _.functions(arrayList);
    test.ok(_.includes(api, 'addAsync'), 'Expected `addAsync` to be present, but it is NOT.');
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
    test.strictEqual(arrayList.size(), 0);
    test.done();
  },

  testStaticAPI: function(test) {
    test.expect(6);
    var String = java.import("java.lang.String");
    test.ok(String);

    var api = _.functions(String);
    test.ok(_.includes(api, 'format'), 'Expected `format` to be present, but it is NOT.');
    test.ok(_.includes(api, 'formatAsync'), 'Expected `formatAsync` to be present, but it is NOT.');
    test.ok(!_.includes(api, 'formatSync'), 'Expected `formatSync` to NOT be present, but it is.');
    test.ok(!_.includes(api, 'formatPromise'), 'Expected `formatPromise` to NOT be present, but it is.');
    test.ok(!_.includes(api, 'formatundefined'), 'Expected `formatundefined` to NOT be present, but it is.');
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

  // See testUnusableMethodName.js for the purpose of these last two tests.
  // In that test, Test.name_alt() is an async method.
  // In this test, it is a sync method.
  testUnusableMethodNameThrows: function(test) {
    test.expect(1);
    var Test = java.import("Test");
    test.ok(Test);
    test.throws(
      function() {
        Test.name();
      },
      function(err) {
        if (err instanceof TypeError) {
          test.done();
          return true;
        } else {
          test.done(err);
          return false;
        }
      }
    );
  },

  testAlternateMethodNameWorks: function(test) {
    test.expect(4);
    var Test = java.import("Test");
    test.ok(Test);
    test.strictEqual(Test.name_alt(), "name");
    test.strictEqual(Test.caller_alt(), "caller");
    test.strictEqual(Test.arguments_alt(), "arguments");
    test.done();
  },

  testReservedFieldName: function(test) {
    test.expect(7);
    var TestEnum = java.import("Test$Enum");
    test.ok(TestEnum);

    // 'foo' and 'bar' are valid enum names
    test.strictEqual(TestEnum.foo.toString(), "foo");
    test.strictEqual(TestEnum.bar.toString(), "bar");

    // TestEnum.name is actually the name of the proxy constructor function.
    test.strictEqual(TestEnum.name, "javaClassConstructorProxy");

    // Instead we need to acccess TestEnum.name_alt
    test.strictEqual(TestEnum.name_alt.toString(), "name");
    test.strictEqual(TestEnum.caller_alt.toString(), "caller");
    test.strictEqual(TestEnum.arguments_alt.toString(), "arguments");

    test.done();
  },
}
