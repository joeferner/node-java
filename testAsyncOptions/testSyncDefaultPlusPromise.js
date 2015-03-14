// testSyncDefaultPlusPromise.js

// Just Sync and Promise, with Sync the default (i.e. no suffix).
// This is the configuration that RedSeal wants for use with Tinkerpop/Gremlin.

var java = require("../");
var assert = require("assert");
var _ = require('lodash');

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

    var api = _.functions(arrayList);
    test.ok(_.includes(api, 'add'), 'Expected `add` to be present, but it is NOT.');
    test.ok(_.includes(api, 'addP'), 'Expected `addP` to be present, but it is NOT.');
    test.ok(!_.includes(api, 'addSync'), 'Expected `addSync` to NOT be present, but it is.');
    test.ok(!_.includes(api, 'addAsync'), 'Expected `addAsync` to NOT be present, but it is.');
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
