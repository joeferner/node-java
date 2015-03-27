// testInvalidLaunch.js

var _ = require('lodash');
var java = require("../");
var nodeunit = require("nodeunit");

module.exports = {

  failedLaunch: function(test) {
    test.expect(3);
    test.ok(!java.isJvmCreated());

    java.asyncOptions = {
      syncSuffix: "Sync",
      asyncSuffix: ""
    };

    // First show that if asyncOptions.promisify is undefined, using the promise variant of ensureJvm throws an error.
    test.throws(function() { java.ensureJvm(); }, Error, /requires its one argument to be a callback function/);

    test.ok(!java.isJvmCreated());
    test.done();
  },

  callbackNotAFunction: function(test) {
    test.expect(3);
    test.ok(!java.isJvmCreated());

    java.asyncOptions = {
      syncSuffix: "",
      promiseSuffix: 'P',
      promisify: require('when/node').lift         // https://github.com/cujojs/when
    };

    test.throws(function() { java.ensureJvm('foo'); }, Error, /requires its one argument to be a callback function/);

    test.ok(!java.isJvmCreated());
    test.done();
  },

  jvmCanStillBeLaunched: function(test) {
    // None of the previous tests should have caused the JVM to be created, so it should still be possible to create one.

    test.expect(2);
    test.ok(!java.isJvmCreated());

    java.asyncOptions = {
      syncSuffix: "",
      promiseSuffix: 'P',
      promisify: require('when/node').lift         // https://github.com/cujojs/when
    };

    java.ensureJvm().done(function() {
      test.ok(java.isJvmCreated());
      test.done();
    });
  }

}
