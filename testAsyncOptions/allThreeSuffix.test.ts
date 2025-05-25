// All three variants have non-empty suffix, i.e a suffix is required for any variant.

import { Java } from "../java";
import { getJava } from "../testHelpers";
import { describe, test, expect, beforeAll } from "vitest";

describe("allThreeSuffix", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava({
      syncSuffix: "Sync",
      asyncSuffix: "Async",
      promiseSuffix: "Promise",
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      promisify: ((await import("when/node")) as any).lift, // https://github.com/cujojs/when
    });
  });

  test("api", () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    expect(arrayList).toBeDefined();
    expect(java.instanceOf(arrayList, "java.util.ArrayList")).toBeTruthy();

    expect(typeof arrayList.addSync !== "undefined", "Expected `addSync` to be present, but it is NOT.").toBeTruthy();
    expect(typeof arrayList.addAsync !== "undefined", "Expected `addAsync` to be present, but it is NOT.").toBeTruthy();
    expect(
      typeof arrayList.addPromise !== "undefined",
      "Expected `addPromise` to be present, but it is NOT."
    ).toBeTruthy();
    expect(typeof arrayList.add === "undefined", "Expected `add` to NOT be present, but it is.").toBeTruthy();
  });

  test("importClass", () => {
    // Note: java.import executes javascript code in src-node/nodeJavaBridge that makes sync calls to java classes.
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
    expect(api.includes("formatSync"), "Expected `formatSync` to be present, but it is NOT.").toBeTruthy();
    expect(api.includes("formatAsync"), "Expected `formatAsync` to be present, but it is NOT.").toBeTruthy();
    expect(api.includes("formatPromise"), "Expected `formatPromise` to be present, but it is NOT.").toBeTruthy();
    expect(!api.includes("format"), "Expected `format` to NOT be present, but it is.").toBeTruthy();
    expect(!api.includes("formatundefined"), "Expected `formatundefined` to NOT be present, but it is.").toBeTruthy();
  });

  test("syncCalls", () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.addSync("hello");
    arrayList.addSync("world");
    expect(arrayList.sizeSync()).toBe(2);
  });

  test("staticSyncCalls", () => {
    // Note: java.import executes javascript code in src-node/nodeJavaBridge that makes sync calls to java classes.
    // Among other things, java.import creates Sync functions for static methods.
    const String = java.import("java.lang.String");
    expect(String.formatSync("%s--%s", "hello", "world")).toBe("hello--world");
  });

  test("asyncCalls", async () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    await new Promise<void>((resolve) => {
      arrayList.addAsync("hello", (err: Error | undefined) => {
        expect(err).toBeUndefined();
        arrayList.addAsync("world", (err: Error | undefined) => {
          expect(err).toBeUndefined();
          arrayList.sizeAsync((err: Error | undefined, size: number) => {
            expect(err).toBeUndefined();
            expect(size).toBe(2);
            resolve();
          });
        });
      });
    });
  });

  test("promiseCalls", async () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    await arrayList
      .addPromise("hello")
      .then(() => {
        return arrayList.addPromise("world");
      })
      .then(() => {
        return arrayList.sizePromise();
      })
      .then((size: number) => {
        expect(size).toBe(2);
      });
  });
});
