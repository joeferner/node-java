// The defacto case but with promises also enabled.

import { beforeAll, describe, expect, test } from "vitest";
import { Java } from "../java";
import { getJava } from "../testHelpers";

describe("defactoPlusPromise", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava(
      {
        syncSuffix: "Sync",
        asyncSuffix: "",
        promiseSuffix: "Promise",
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        promisify: ((await import("when/node")) as any).lift, // https://github.com/cujojs/when
      },
      {
        beforeInit: (java) => {
          expect(java.isJvmCreated()).toBeFalsy();

          function before(callback: () => void): void {
            expect(java.isJvmCreated()).toBeFalsy();
            callback();
          }

          function after(callback: () => void): void {
            expect(java.isJvmCreated()).toBeTruthy();
            callback();
          }

          java.registerClient(before, after);
          java.registerClient(null, after);
          java.registerClient(before);
        },
      }
    );

    await java.ensureJvm();
    expect(java.isJvmCreated()).toBeTruthy();
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
    // Note: java.import executes javascript code in src-node/nodeJavaBridge that makes sync calls to java classes.
    // Among other things, java.import creates Sync functions for static methods.
    const String = java.import("java.lang.String");
    expect(String.formatSync("%s--%s", "hello", "world")).toBe("hello--world");
  });

  test("asyncCalls", async () => {
    await new Promise<void>((resolve) => {
      const arrayList = java.newInstanceSync("java.util.ArrayList");
      arrayList.add("hello", (err: Error | undefined) => {
        expect(err).toBeFalsy();
        arrayList.add("world", (err: Error | undefined) => {
          expect(err).toBeFalsy();
          arrayList.size((err: Error | undefined, size: number | undefined) => {
            expect(err).toBeFalsy();
            expect(size).toBe(2);
            resolve();
          });
        });
      });
    });
  });

  test("promiseCalls", async () => {
    await new Promise<void>((resolve) => {
      const arrayList = java.newInstanceSync("java.util.ArrayList");
      arrayList
        .addPromise("hello")
        .then(() => {
          return arrayList.addPromise("world");
        })
        .then(() => {
          return arrayList.sizePromise();
        })
        .then((size: number) => {
          expect(size).toBe(2);
          resolve();
        });
    });
  });
});
