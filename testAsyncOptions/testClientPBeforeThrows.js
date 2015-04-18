// testClientPBeforeThrows.js

var _ = require('lodash');
var java = require("../");
var nodeunit = require("nodeunit");
var when = require('when');

module.exports = {

  clientPBeforeThrows: function(test) {
    test.expect(6);
    test.ok(!java.isJvmCreated());

    java.asyncOptions = {
      syncSuffix: "Sync",
      promiseSuffix: 'Promise',
      promisify: require('when/node').lift         // https://github.com/cujojs/when
    };

    function beforeP() {
      var promise = when.promise(function(resolve, reject) {
        test.ok(!java.isJvmCreated());
        throw new Error('dummy error');
      });
      return promise;
    }

    java.registerClientP(beforeP);

    java.ensureJvm().done(
      function () {
        test.ok(false);
      },
      function(err) {
        test.ok(_.isObject(err));
        test.ok(err instanceof Error);
        test.strictEqual(err.message, 'dummy error');
        test.ok(!java.isJvmCreated());
        test.done();
      }
    );
  }

}
