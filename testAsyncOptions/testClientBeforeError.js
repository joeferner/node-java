// testClientBeforeError.js

var java = require("../");
var nodeunit = require("nodeunit");

module.exports = {

  clientBeforeError: function(test) {
    test.expect(6);
    test.ok(!java.isJvmCreated());

    java.asyncOptions = {
      syncSuffix: "Sync",
    };

    function before(callback) {
      test.ok(!java.isJvmCreated());
      callback(new Error('dummy error'));
    }

    java.registerClient(before);

    java.ensureJvm(function(err) {
      test.ok(err && typeof err === 'object');
      test.ok(err instanceof Error);
      test.strictEqual(err.message, 'dummy error');
      test.ok(!java.isJvmCreated());
      test.done();
    });
  }

}
