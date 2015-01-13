var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

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
          test.equal(name, "java.util.ArrayList");
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
        test.equals(100, result);
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
        test.equal(name, "java.util.ArrayList");
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
    var list;
    var it;
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
        test.equal(name, "java.util.ArrayList");
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
        test.equal(val, 'hello');
        return it.nextPromise();
      })
      .then(function(val) {
        test.ok(val);
        test.equal(val, 'world');
        return it.nextPromise();
      })
      .then(function(val) {
        test.ok(val);
        test.equal(val, 'boo');
        return it.hasNextPromise();
      })
      .catch(function(err) {
        test.ifError(err);
      })
      .then(function(more) {
        test.ok(!more);
        return it.nextPromise();
      })
      .catch(function(err) {
        test.ok(/java\.util\.NoSuchElementException/.test(err.toString()));
      })
      .then(function() {
        test.expect(12);
        test.done();
      });
  },

  "test when argument promise resolution": function (test) {
    var when = require('when');
    var list;
    var it;
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
        test.equal(name, "java.util.ArrayList");
      })
      .then(function() {
        list.addPromise(when('hello'));   // note: passing a promise argument to a java method!
      })
      .then(function() {
        list.addPromise(when('world'));
      })
      .then(function() {
        list.addPromise(when('boo'));
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
        test.equal(val, 'hello');
        return it.nextPromise();
      })
      .then(function(val) {
        test.ok(val);
        test.equal(val, 'world');
        return it.nextPromise();
      })
      .then(function(val) {
        test.ok(val);
        test.equal(val, 'boo');
        return it.hasNextPromise();
      })
      .catch(function(err) {
        test.ifError(err);
      })
      .then(function(more) {
        test.ok(!more);
        return it.nextPromise();
      })
      .catch(function(err) {
        test.ok(/java\.util\.NoSuchElementException/.test(err.toString()));
      })
      .then(function() {
        test.expect(12);
        test.done();
      });
  }
});

