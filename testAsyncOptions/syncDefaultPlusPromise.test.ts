// Just Sync and Promise, with Sync the default (i.e. no suffix).
// This is the configuration that RedSeal wants for use with Tinkerpop/Gremlin.

import { beforeAll, describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";
import { Java } from "../java";

describe("syncDefaultPlusPromise", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava({
      syncSuffix: "",
      promiseSuffix: "P",
    });
  });

  test("api", () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    expect(arrayList).toBeTruthy();
    expect(java.instanceOf(arrayList, "java.util.ArrayList")).toBeTruthy();

    expect(typeof arrayList.add !== "undefined", "Expected `add` to be present, but it is NOT.").toBeTruthy();
    expect(typeof arrayList.addP !== "undefined", "Expected `addP` to be present, but it is NOT.").toBeTruthy();
    expect(typeof arrayList.addSync === "undefined", "Expected `addSync` to NOT be present, but it is.").toBeTruthy();
    expect(typeof arrayList.addAsync === "undefined", "Expected `addAsync` to NOT be present, but it is.").toBeTruthy();
  });

  test("importClass", () => {
    // Note: java.import executes javascript code in src-node/nodeJavaBridge that makes sync calls to java classes.
    // This test verifies the import runs without error.
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
    expect(api.includes("formatP"), "Expected `formatP` to be present, but it is NOT.").toBeTruthy();
    expect(api.includes("formatSync"), "Expected `formatSync` to NOT be present, but it is.").toBeFalsy();
    expect(api.includes("formatAsync"), "Expected `formatAsync` to NOT be present, but it is.").toBeFalsy();
    expect(api.includes("formatundefined"), "Expected `formatundefined` to NOT be present, but it is.").toBeFalsy();
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

  test("promiseCalls", async () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    await new Promise<void>((resolve) => {
      arrayList
        .addP("hello")
        .then(() => {
          return arrayList.addP("world");
        })
        .then(() => {
          return arrayList.sizeP();
        })
        .then((size: number) => {
          expect(size).toBe(2);
          resolve();
        });
    });
  });
});
