// testRunner.js

// This is a custom test runner. All tests are run with vitest, but in separate
// processes, which allows us to test java with different configuration options.

const async = require("async");
const chalk = require("chalk");
const childProcess = require("node:child_process");
const glob = require("glob");
const path = require("node:path");

const tests = glob
  .sync("*.test.js", { cwd: path.join(__dirname, "..", "testAsyncOptions") })
  .sort((a, b) => a.toLocaleLowerCase().localeCompare(b.toLocaleLowerCase()));

tests.unshift("test"); // Arrange to run the primary tests first, in a single process

function runTest(testArgs, done) {
  const vitest = path.join("node_modules", ".bin", "vitest");
  const cmd = testArgs === "test" ? `vitest --dir test` : `${vitest} ${testArgs}`;
  console.log(`running "${cmd}"...`);
  childProcess.exec(cmd, (error, stdout, stderr) => {
    const errText = stderr.toString();
    if (errText !== "") {
      console.error(chalk.bold.red(errText));
    }

    process.stdout.write(stdout.toString());
    done(error);
  });
}

console.log('test to run', tests);
async.eachSeries(tests, runTest, (err) => {
  if (err) {
    console.error(chalk.bold.red(err));
    process.exit(1);
    return;
  }
  console.log(chalk.green("Tests completed successfully!"));
});
