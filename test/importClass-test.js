'use strict';

var java = require("../testHelpers").java;
var nodeunit = require("nodeunit");
var util = require("util");

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
    var TestEnum = java.import('Test$Enum');

    // 'foo' and 'bar' are valid enum names
    test.strictEqual(TestEnum.foo.toStringSync(), "foo");
    test.strictEqual(TestEnum.bar.toStringSync(), "bar");

    test.throws(
      function() {
        // The enum also defines 'name', but Javascript prevents us from using it,
        // since Function.name is always an unwritable property.
        var x = TestEnum.name.toStringSync();
      },
      TypeError
    );
    test.done();
  },

  "import TestEnum and use alternate name": function (test) {
    var TestEnum = java.import('Test$Enum');

    // 'foo' and 'bar' are valid enum names
    test.strictEqual(TestEnum.foo.toStringSync(), "foo");
    test.strictEqual(TestEnum.bar.toStringSync(), "bar");

    // 'name' is not, so we must use 'name_' to reference the enum.
    // But note that it's value is still "name".
    test.strictEqual(TestEnum.name_.toStringSync(), "name");
    test.done();
  }

});
