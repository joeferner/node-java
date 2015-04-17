'use strict';

var java = require('../testHelpers').java;

var nodeunit = require('nodeunit');
var util = require('util');

exports['varargs'] = nodeunit.testCase({

  'array signature inferrence': function(test) {
    test.expect(9);
    var Test = java.import('Test');
    test.strictEqual(Test.varArgsSignatureSync([]), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync(['a', 'b']), 'String...');
    test.strictEqual(Test.varArgsSignatureSync([1, 2]), 'Integer...');
    test.strictEqual(Test.varArgsSignatureSync([1.1, 2]), 'Number...');
    test.strictEqual(Test.varArgsSignatureSync([1.1, 'a']), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync([true, 'a']), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync([true, 1]), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync([true, 1.1]), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync([true, false]), 'Boolean...');
    test.done();
  },

  'variadic signature inferrence': function(test) {
    test.expect(9);
    var Test = java.import('Test');
    test.strictEqual(Test.varArgsSignatureSync(), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync('a', 'b'), 'String...');
    test.strictEqual(Test.varArgsSignatureSync(1, 2), 'Integer...');
    test.strictEqual(Test.varArgsSignatureSync(1.1, 2), 'Number...');
    test.strictEqual(Test.varArgsSignatureSync(1.1, 'a'), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync(true, 'a'), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync(true, 1), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync(true, 1.1), 'Object...');
    test.strictEqual(Test.varArgsSignatureSync(true, false), 'Boolean...');
    test.done();
  },

  'variadic no args': function(test) {
    test.expect(1);
    var String = java.import('java.lang.String');
    test.strictEqual(String.formatSync('nothing'), 'nothing');
    test.done();
  },

  'variadic one args': function(test) {
    test.expect(1);
    var String = java.import('java.lang.String');
    test.strictEqual(String.formatSync('%s', 'hello'), 'hello');
    test.done();
  },

  'variadic two args': function(test) {
    test.expect(1);
    var String = java.import('java.lang.String');
    test.strictEqual(String.formatSync('%s--%s', 'hello', 'world'), 'hello--world');
    test.done();
  },

  'newArray(Object) no args passed': function(test) {
    test.expect(1);
    var String = java.import('java.lang.String');
    test.strictEqual(String.formatSync('nothing', java.newArray('java.lang.Object', [])), 'nothing');
    test.done();
  },

  'newArray(Object) one args': function(test) {
    test.expect(1);
    var String = java.import('java.lang.String');
    test.strictEqual(String.formatSync('%s', java.newArray('java.lang.Object', ['hello'])), 'hello');
    test.done();
  },

  'newArray(Object) two args': function(test) {
    test.expect(1);
    var String = java.import('java.lang.String');
    test.strictEqual(String.formatSync('%s--%s', java.newArray('java.lang.Object', ['hello', 'world'])), 'hello--world');
    test.done();
  },

  'Call static method with variadic varargs': function(test) {
    test.expect(4);
    var Test = java.import('Test');
    test.equal(Test.staticVarargsSync(5), '5');
    test.equal(Test.staticVarargsSync(5, 'a'), '5a');
    test.equal(Test.staticVarargsSync(5, 'a', 'b'), '5ab');
    test.equal(Test.staticVarargsSync(5, 'a', 'b', 'c'), '5abc');
    test.done();
  },

  'Call static varargs method with plain array': function(test) {
    test.expect(3);
    var Test = java.import('Test');
    test.equal(Test.staticVarargsSync(5, ['a']), '5a');
    test.equal(Test.staticVarargsSync(5, ['a', 'b']), '5ab');
    test.equal(Test.staticVarargsSync(5, ['a', 'b', 'c']), '5abc');
    test.done();
  },

  'Call static varags method with newArray': function(test) {
    test.expect(2);
    var Test = java.import('Test');
    test.equal(Test.staticVarargsSync(5, java.newArray('java.lang.String', ['a'])), '5a');
    test.equal(Test.staticVarargsSync(5, java.newArray('java.lang.String', ['a', 'b', 'c'])), '5abc');
    test.done();
  }

});
