// testClientPBeforeError.js

var java = require("../");
var nodeunit = require("nodeunit");
var when = require('when');

module.exports = {

  clientPBeforeError: function(test) {
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
        reject(new Error('dummy error'));
      });
      return promise;
    }

    java.registerClientP(beforeP);

    java.ensureJvm().done(
      function () {
        test.ok(false);
      },
      function(err) {
        test.ok(err && typeof err === 'object');
        test.ok(err instanceof Error);
        test.strictEqual(err.message, 'dummy error');
        test.ok(!java.isJvmCreated());
        test.done();
      }
    );
  }

}
