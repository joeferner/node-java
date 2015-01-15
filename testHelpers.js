var java = require("./");
java.options.push("-Djava.awt.headless=true");
//java.options.push('-agentlib:jdwp=transport=dt_socket,server=y,suspend=y,address=5005');

java.classpath.push("test/");
java.classpath.push("test/commons-lang3-3.1.jar");
java.classpath.push("test8/");

function promisifyQ(f) {
  // Q doesn't provide a promisify function that works directly on a method.
  // The .denodeify() (aka .nfbind()) function requires a bound function.
  return function(/* arguments */) {
    return require('q').nbind(f, this).apply(undefined, arguments);
  }
}

java.asyncOptions = {
  promiseSuffix: 'Promise',
  promisify: require('when/node').lift         // https://github.com/cujojs/when

// We've tested with 5 different Promises/A+ implementations:
//   promisify: require('bluebird').promisify     // https://github.com/petkaantonov/bluebird/
//   promisify: require('promise').denodeify      // https://github.com/then/promise
//   promisify: require('vow-node').promisify     // https://github.com/dfilatov/vow-node
//   promisify: require('when/node').lift         // https://github.com/cujojs/when
//   promisify: promisifyQ                        // https://github.com/kriskowal/q requires wrapper promisifyQ.
};

module.exports.java = java;
