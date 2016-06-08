// testRunner.js

// This is a custom test runner. All tests are run with nodeunit, but in separate
// processes, which allows us to test java with different configuration options.

var async = require('async');
var chalk = require('chalk');
var childProcess = require('child_process');
var glob = require('glob');
var path = require('path');

var tests = glob.sync(path.join('testAsyncOptions', '*.js'));

tests.unshift('test test8');  // Arrange to run the primary tests first, in a single process

function runTest(testArgs, done) {
  var cmd = 'node_modules/.bin/nodeunit ';
  if(process.platform == "win32")
    cmd = 'node_modules\\.bin\\nodeunit ';  
  cmd += testArgs;
  childProcess.exec(cmd, function (error, stdout, stderr) {
    // It appears that nodeunit merges error output into the stdout
    // so these three lines are probably useless.
    var errText = stderr.toString();
    if (errText !== '')
      console.error(chalk.bold.red(errText));

    process.stdout.write(stdout.toString());
    done(error);
  });
}

async.eachSeries(tests, runTest, function(err) {
  if (err) {
    console.error(chalk.bold.red(err));
    process.exit(1);
  }
});
