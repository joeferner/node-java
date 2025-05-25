import { beforeAll, describe, expect, test } from "vitest";
import { expectJavaError, getJava } from "../testHelpers";
import { Java } from "../java";

describe("Java8", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava();
  });

  test("call methods of a class that uses lambda expressions", () => {
    try {
      const TestLambda = java.import("TestLambda");
      const lambda = new TestLambda();
      const sum = lambda.testLambdaAdditionSync(23, 42);
      expect(sum).toBe(65);
      const diff = lambda.testLambdaSubtractionSync(23, 42);
      expect(diff).toBe(-19);
    } catch (err) {
      expectJavaError(err);
      const unsupportedVersion = java.instanceOf(err.cause, "java.lang.UnsupportedClassVersionError");
      if (unsupportedVersion) {
        console.log("JRE 1.8 not available");
      } else {
        console.error("Java8 test failed with unknown error:", err);
        throw err;
      }
    }
  });
});
