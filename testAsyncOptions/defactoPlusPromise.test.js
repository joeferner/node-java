// The defacto case but with promises also enabled.

import { beforeAll, describe, expect, test } from "vitest";
import { java } from "../testHelpers";

describe("defactoPlusPromise", () => {
  beforeAll(async () => {
    await new Promise((resolve) => {
      const api = Object.keys(java).filter((key) => typeof java[key] === "function");
      expect(api.includes("isJvmCreated"), "Expected `isJvmCreated` to be present, but it is NOT.").toBeTruthy();
      expect(java.isJvmCreated()).toBeFalsy();

      java.asyncOptions = {
        syncSuffix: "Sync",
        asyncSuffix: "",
        promiseSuffix: "Promise",
        promisify: require("when/node").lift, // https://github.com/cujojs/when
      };

      function before(callback) {
        expect(java.isJvmCreated()).toBeFalsy();
        callback();
      }

      function after(callback) {
        expect(java.isJvmCreated()).toBeTruthy();
        callback();
      }

      java.registerClient(before, after);
      java.registerClient(null, after);
      java.registerClient(before);

      java.ensureJvm().done(function () {
        expect(java.isJvmCreated()).toBeTruthy();
        resolve();
      });
    });
  });

  test("api", () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    expect(arrayList).toBeTruthy();
    expect(java.instanceOf(arrayList, "java.util.ArrayList")).toBeTruthy();

    expect(arrayList.addSync !== "undefined", "Expected `addSync` to be present, but it is NOT.").toBeTruthy();
    expect(arrayList.add !== "undefined", "Expected `add` to be present, but it is NOT.").toBeTruthy();
    expect(arrayList.addPromise !== "undefined", "Expected `addPromise` to be present, but it is NOT.").toBeTruthy();
  });

  test("importClass", () => {
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    const ArrayList = java.import("java.util.ArrayList");
    expect(ArrayList).toBeTruthy();
    const arrayList = new ArrayList();
    expect(arrayList).toBeTruthy();
    expect(arrayList.sizeSync()).toBe(0);
  });

  test("staticAPI", () => {
    const String = java.import("java.lang.String");
    expect(String).toBeTruthy();

    const api = Object.keys(String).filter((key) => typeof String[key] === "function");
    expect(api.includes("format"), "Expected `format` to be present, but it is NOT.").toBeTruthy();
    expect(api.includes("formatSync"), "Expected `formatSync` to be present, but it is NOT.").toBeTruthy();
    expect(api.includes("formatPromise"), "Expected `formatPromise` to be present, but it is NOT.").toBeTruthy();
    expect(api.includes("formatAsync"), "Expected `formatAsync` to NOT be present, but it is.").toBeFalsy();
    expect(api.includes("formatundefined"), "Expected `formatundefined` to NOT be present, but it is.").toBeFalsy();
  });

  test("syncCalls", () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.addSync("hello");
    arrayList.addSync("world");
    expect(arrayList.sizeSync()).toBe(2);
  });

  test("staticSyncCalls", () => {
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    // Among other things, java.import creates Sync functions for static methods.
    const String = java.import("java.lang.String");
    expect(String.formatSync("%s--%s", "hello", "world")).toBe("hello--world");
  });

  test("asyncCalls", async () => {
    await new Promise((resolve) => {
      const arrayList = java.newInstanceSync("java.util.ArrayList");
      arrayList.add("hello", function (err) {
        expect(err).toBeFalsy();
        arrayList.add("world", function (err) {
          expect(err).toBeFalsy();
          arrayList.size(function (err, size) {
            expect(err).toBeFalsy();
            expect(size).toBe(2);
            resolve();
          });
        });
      });
    });
  });

  test("promiseCalls", async () => {
    await new Promise((resolve) => {
      const arrayList = java.newInstanceSync("java.util.ArrayList");
      arrayList
        .addPromise("hello")
        .then(() => {
          return arrayList.addPromise("world");
        })
        .then(() => {
          return arrayList.sizePromise();
        })
        .then((size) => {
          expect(size).toBe(2);
          resolve();
        });
    });
  });
});
