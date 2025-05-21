// Use "Async" for the asyncSuffix, and "" for the syncSuffix.

import { beforeAll, describe, expect, test } from "vitest";
import { java } from "../testHelpers";

describe("asyncSuffixSyncDefault", () => {
  beforeAll(async () => {
    await new Promise((resolve) => {
      java.asyncOptions = {
        syncSuffix: "",
        asyncSuffix: "Async",
        ifReadOnlySuffix: "_alt",
      };

      function before(callback) {
        java.classpath.push("test/");
        expect(java.isJvmCreated()).toBeFalsy();
        callback();
      }

      function after(callback) {
        expect(java.isJvmCreated()).toBeTruthy();
        callback();
      }

      java.registerClient(before, after);

      java.ensureJvm(function (err) {
        expect(err).toBeNull();
        expect(java.isJvmCreated()).toBeTruthy();
        resolve();
      });
    });
  });

  test("api", () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    expect(arrayList).toBeTruthy();
    expect(java.instanceOf(arrayList, "java.util.ArrayList")).toBeTruthy();

    expect(typeof arrayList.addAsync !== "undefined", "Expected `addAsync` to be present, but it is NOT.").toBeTruthy();
    expect(typeof arrayList.add !== "undefined", "Expected `add` to be present, but it is NOT.").toBeTruthy();
    expect(
      typeof arrayList.addPromise === "undefined",
      "Expected `addPromise` to NOT be present, but it is."
    ).toBeTruthy();
  });

  test("importClass", () => {
    // Note: java.import executes javascript code in src-node/nodeJavaBridge that makes sync calls to java classes.
    const ArrayList = java.import("java.util.ArrayList");
    expect(ArrayList).toBeTruthy();
    const arrayList = new ArrayList();
    expect(arrayList).toBeTruthy();
    expect(arrayList.size()).toBe(0);
  });

  test("staticAPI", () => {
    const String = java.import("java.lang.String");
    expect(String).toBeTruthy();

    const api = Object.keys(String).filter((key) => typeof String[key] === "function");
    expect(api.includes("format"), "Expected `format` to be present, but it is NOT.").toBeTruthy();
    expect(api.includes("formatAsync"), "Expected `formatAsync` to be present, but it is NOT.").toBeTruthy();
    expect(!api.includes("formatSync"), "Expected `formatSync` to NOT be present, but it is.").toBeTruthy();
    expect(!api.includes("formatPromise"), "Expected `formatPromise` to NOT be present, but it is.").toBeTruthy();
    expect(!api.includes("formatundefined"), "Expected `formatundefined` to NOT be present, but it is.").toBeTruthy();
  });

  test("syncCalls", () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.add("hello");
    arrayList.add("world");
    expect(arrayList.size()).toBe(2);
  });

  test("staticSyncCalls", () => {
    // Note: java.import executes javascript code in src-node/nodeJavaBridge that makes sync calls to java classes.
    // Among other things, java.import creates Sync functions for static methods.
    const String = java.import("java.lang.String");
    expect(String.format("%s--%s", "hello", "world")).toBe("hello--world");
  });

  test("asyncCalls", async () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    await new Promise((resolve) => {
      arrayList.addAsync("hello", function (err) {
        expect(err).toBeUndefined();
        arrayList.addAsync("world", function (err) {
          expect(err).toBeUndefined();
          arrayList.sizeAsync(function (err, size) {
            expect(err).toBeUndefined();
            expect(size).toBe(2);
            resolve();
          });
        });
      });
    });
  });

  // See testUnusableMethodName.js for the purpose of these last two tests.
  // In that test, Test.name_alt() is an async method.
  // In this test, it is a sync method.
  test("unusableMethodNameThrows", () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    expect(() => Test.name()).toThrowError(TypeError);
  });

  test("alternateMethodNameWorks", () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    expect(Test.name_alt()).toBe("name");
    expect(Test.caller_alt()).toBe("caller");
    expect(Test.arguments_alt()).toBe("arguments");
  });

  test("reservedFieldName", () => {
    const TestEnum = java.import("Test$Enum");
    expect(TestEnum).toBeTruthy();

    // 'foo' and 'bar' are valid enum names
    expect(TestEnum.foo.toString()).toBe("foo");
    expect(TestEnum.bar.toString()).toBe("bar");

    // TestEnum.name is actually the name of the proxy constructor function.
    expect(TestEnum.name).toBe("javaClassConstructorProxy");

    // Instead we need to access TestEnum.name_alt
    expect(TestEnum.name_alt.toString()).toBe("name");
    expect(TestEnum.caller_alt.toString()).toBe("caller");
    expect(TestEnum.arguments_alt.toString()).toBe("arguments");
  });
});
