var java = require("./");
java.options.push("-Djava.awt.headless=true");
//java.options.push('-agentlib:jdwp=transport=dt_socket,server=y,suspend=y,address=5005');

java.classpath.push("test/");
java.classpath.push("test/commons-lang3-3.1.jar");
java.classpath.push("test8/");

java.asyncOptions = {
  promiseSuffix: 'Promise',
  promisify: require('when/node').lift         // when works with all three node versions

// PASSES in all three node versions: 0.8.28, 0.10.35, 0.11.14
//   promisify: require('when/node').lift         // when works with all three node versions
//   promisify: require('promise').denodeify      // promise works with all three node versions
//   promisify: require('vow-node').promisify     // vow-node works with all three node versions

// PASSES in Node 0.10, 0.11.   (incompatible with Node 0.8).
//   promisify: require('bluebird').promisify     // bluebird requires node >=0.10

// FAILS:
//   promisify: require('q').denodeify            // FAILS: Q triggers assertion failure in node_object_wrap.h, line 61
//   promisify: require('p-promise').denodeify    // FAILS: P-promise does not implement catch().
};

module.exports.java = java;
