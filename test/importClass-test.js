'use strict';

var java = require("../testHelpers").java;
var nodeunit = require("nodeunit");
var util = require("util");
var _ = require("lodash");

exports['Import Class'] = nodeunit.testCase({
  tearDown: function (callback) {
    java.setStaticFieldValue("Test", "staticFieldInt", 42);
    callback();
  },

  "import": function (test) {
    var Test = java.import('Test');
    test.equals(42, Test.staticFieldInt);
    Test.staticFieldInt = 200;
    test.equals(200, Test.staticFieldInt);

    test.equals(100, Test.staticMethodSync(99));
    Test.staticMethod(99, function (err, result) {
      test.ok(!err);
      test.equals(100, result);

      var testObj = new Test(5);
      test.equals(5, testObj.getIntSync());
      test.done();
    });
  },

  "import TestEnum with unsable name": function (test) {
    test.expect(5);
    var TestEnum = java.import('Test$Enum');

    // 'foo' and 'bar' are valid enum names
    test.strictEqual(TestEnum.foo.toStringSync(), "foo");
    test.strictEqual(TestEnum.bar.toStringSync(), "bar");

    _.forEach(['name', 'arguments', 'caller'], function(prop) {
      test.throws(
        function() {
          // The enum also defines 'name', 'caller', and 'attributes', but Javascript prevents us from using them,
          // since these are unwritable properties of Function.
          var x = TestEnum[prop].toStringSync();
        },
        TypeError
      );
    });
    test.done();
  },

  "import TestEnum and use alternate name": function (test) {
    test.expect(5);
    var TestEnum = java.import('Test$Enum');

    // 'foo' and 'bar' are valid enum names
    test.strictEqual(TestEnum.foo.toStringSync(), "foo");
    test.strictEqual(TestEnum.bar.toStringSync(), "bar");

    // 'name', 'caller', and 'arguments' are not, so we must use e.g. 'name_' to reference the enum.
    // But note that the value is still e.g. "name".
    test.strictEqual(TestEnum.name_.toStringSync(), "name");
    test.strictEqual(TestEnum.arguments_.toStringSync(), "arguments");
    test.strictEqual(TestEnum.caller_.toStringSync(), "caller");
    test.done();
  }

});
