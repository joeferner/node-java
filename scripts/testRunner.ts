// testRunner.js

// This is a custom test runner. All tests are run with vitest, but in separate
// processes, which allows us to test java with different configuration options.

import chalk from "chalk";
import findRoot from "find-root";
import * as glob from "glob";
import childProcess, { ExecException } from "node:child_process";
import path from "node:path";

const root = findRoot(__dirname);

const tests = glob
  .sync("*.test.ts", { cwd: path.join(root, "testAsyncOptions") })
  .sort((a: string, b: string) => a.toLocaleLowerCase().localeCompare(b.toLocaleLowerCase()));

tests.unshift("test"); // Arrange to run the primary tests first, in a single process

async function runTest(testIndex: number, testArgs: string): Promise<void> {
  const vitest = path.join("node_modules", ".bin", "vitest");
  const cmd = testArgs === "test" ? `vitest --dir test` : `${vitest} ${testArgs}`;
  console.log(chalk.cyan(`(${testIndex + 1}/${tests.length}) running "${cmd}"...`));
  return new Promise<void>((resolve, reject) => {
    childProcess.exec(cmd, (error: ExecException | null, stdout: string, stderr: string) => {
      const errText = stderr.toString();
      if (errText !== "") {
        console.error(chalk.bold.red(errText));
      }

      process.stdout.write(stdout.toString());
      if (error) {
        return reject(error);
      }
      resolve();
    });
  });
}

console.log("test to run", tests);
async function runAll(): Promise<void> {
  for (let i = 0; i < tests.length; i++) {
    const test = tests[i];
    await runTest(i, test);
  }
}

runAll()
  .then(() => {
    console.log(chalk.green("Tests completed successfully!"));
  })
  .catch((err) => {
    console.error(chalk.bold.red(err));
    process.exit(1);
  });
