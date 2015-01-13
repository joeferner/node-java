var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

function isOpenJDK() {
  var javaVendor = java.callStaticMethodSync('java.lang.System', 'getProperty', 'java.vendor');
  var javaVersion = java.callStaticMethodSync('java.lang.System', 'getProperty', 'java.version');
  return /Sun Microsystems Inc/.test(javaVendor) && /^1\.6/.test(javaVersion);
}

exports['Promises'] = nodeunit.testCase({
  "create an instance of a class and call methods (getClassPromise & getNamePromise)": function(test) {
    // Adapted from a test in simple-test.js
    java.newInstance("java.util.ArrayList", function(err, list) {
      test.ifError(err);
      test.ok(list);
      list.getClassPromise()
        .then(function(clazz) {
          test.ok(clazz);
          return clazz.getNamePromise();
        })
        .then(function(name) {
          test.strictEqual(name, "java.util.ArrayList");
        })
        .catch(function(err) {
          test.ifError(err);
        })
        .then(function() {
          test.expect(4);
          test.done();
        });
    });
  },

  "import and execute promisified static method": function (test) {
    var Test = java.import('Test');
    Test.staticMethodPromise(99)
      .then(function (result) {
        test.strictEqual(100, result);
      })
      .catch(function (err) {
        test.ifError(err);
      })
      .then(function() {
        test.expect(1);
        test.done();
      });
  },

  "run promisified method of Java module (newInstancePromise)": function (test) {
    java.newInstancePromise("java.util.ArrayList")
      .then(function(list) {
        test.ok(list);
        return list.getClassPromise();
      })
      .then(function(clazz) {
        test.ok(clazz);
        return clazz.getNamePromise();
      })
      .then(function(name) {
        test.strictEqual(name, "java.util.ArrayList");
      })
      .catch(function(err) {
        test.ifError(err);
      })
      .then(function() {
        test.expect(3);
        test.done();
      });
  },

  "run chained promisified methods (of class java.util.ArrayList)": function (test) {
    var openJDK = isOpenJDK();
    if (openJDK) {
      // This test exposes a latent node-java bug with OpenJDK 1.6.
      // See https://github.com/joeferner/node-java/issues/186
      // For now, we simply don't run this test on OpenJDK 1.6.
      test.done();
      return;
    }
    var list;
    var it;
    var expectException = false;
    java.newInstancePromise("java.util.ArrayList")
      .then(function(_list) {
        test.ok(_list);
        list = _list;
        return list.getClassPromise();
      })
      .then(function(clazz) {
        test.ok(clazz);
        return clazz.getNamePromise();
      })
      .then(function(name) {
        test.strictEqual(name, "java.util.ArrayList");
      })
      .then(function() {
        list.addPromise('hello');
      })
      .then(function() {
        list.addPromise('world');
      })
      .then(function() {
        list.addPromise('boo');
      })
      .then(function() {
        return list.iteratorPromise();
      })
      .then(function(_it) {
        test.ok(_it);
        it = _it;
        return it.nextPromise();
      })
      .then(function(val) {
        test.ok(val);
        console.log(typeof val, val);
        test.strictEqual(val, 'hello'); // java.lang.InternalError exception thrown here with OpenJDK
        return it.nextPromise();
      })
      .then(function(val) {
        test.ok(val);
        console.log(typeof val, val);
        test.strictEqual(val, 'world');
        return it.nextPromise();
      })
      .then(function(val) {
        test.ok(val);
        console.log(typeof val, val);
        test.strictEqual(val, 'boo');
        return it.hasNextPromise();
      })
      .then(function(more) {
        console.log(typeof more, more);
        test.strictEqual(more, false);
        expectException = true;
        return it.nextPromise();
      })
      .catch(function(err) {
        test.ok(expectException);
      })
      .then(function() {
        test.expect(12);
        test.done();
      });
  }

});

