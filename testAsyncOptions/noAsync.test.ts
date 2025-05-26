// Just Sync and Promise, both with a non-empty suffix.

import { beforeAll, describe, expect, test } from "vitest";
import { Java } from "../java";
import { getJava } from "../testHelpers";

describe("noAsync", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava(
      {
        syncSuffix: "Sync",
        promiseSuffix: "Promise",
      },
      {
        beforeInit: (java) => {
          expect(java.isJvmCreated()).toBeFalsy();

          function beforeP(): Promise<void> {
            return new Promise((resolve) => {
              expect(java.isJvmCreated()).toBeFalsy();
              resolve();
            });
          }

          function afterP(): Promise<void> {
            return new Promise((resolve) => {
              expect(java.isJvmCreated()).toBeTruthy();
              resolve();
            });
          }

          java.registerClientP(beforeP, afterP);
          java.registerClientP(null, afterP);
          java.registerClientP(beforeP);
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

    expect(typeof arrayList.addSync !== "undefined", "Expected `addSync` to be present, but it is NOT.").toBeTruthy();
    expect(
      typeof arrayList.addPromise !== "undefined",
      "Expected `addPromise` to be present, but it is NOT."
    ).toBeTruthy();
    expect(typeof arrayList.add === "undefined", "Expected `add` to NOT be present, but it is.").toBeTruthy();
    expect(typeof arrayList.addAsync === "undefined", "Expected `addAsync` to NOT be present, but it is.").toBeTruthy();
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
    expect(api.includes("formatPromise"), "Expected `formatPromise` to be present, but it is NOT.").toBeTruthy();
    expect(api.includes("format"), "Expected `format` to NOT be present, but it is.").toBeFalsy();
    expect(api.includes("formatAsync"), "Expected `formatAsync` to NOT be present, but it is.").toBeFalsy();
    expect(api.includes("formatundefined"), "Expected `formatundefined` to NOT be present, but it is.").toBeFalsy();
  });

  test("syncCalls", () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.addSync("hello");
    arrayList.addSync("world");
    expect(arrayList.sizeSync()).toBe(2);
  });

  test("sStaticSyncCalls", () => {
    // Note: java.import executes javascript code in src-node/nodeJavaBridge that makes sync calls to java classes.
    // Among other things, java.import creates Sync functions for static methods.
    const String = java.import("java.lang.String");
    expect(String.formatSync("%s--%s", "hello", "world")).toBe("hello--world");
  });

  test("promiseCalls", async () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    await new Promise<void>((resolve) => {
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
