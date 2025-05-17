// In the defacto case, the developer sets asyncOptions, but specifies the defacto standard behavior.

import { beforeAll, describe, expect, test } from "vitest";
import { java } from "../testHelpers";

describe('defacto', () => {
  beforeAll(async () => {
    await new Promise(resolve => {
      const api = Object.keys(java).filter((key) => typeof java[key] === 'function');
      expect(api.includes('isJvmCreated'), 'Expected `isJvmCreated` to be present, but it is NOT.').toBeTruthy();
      expect(java.isJvmCreated()).toBeFalsy();

      java.asyncOptions = {
        syncSuffix: "Sync",
        asyncSuffix: ""
      };

      function before() {
        expect(java.isJvmCreated()).toBeFalsy();
      }

      function after() {
        expect(java.isJvmCreated()).toBeTruthy();
      }

      java.registerClient(before, after);
      java.registerClient(undefined, after);
      java.registerClient(before, undefined);

      java.ensureJvm(function (err) {
        expect(err).toBeFalsy();
        expect(java.isJvmCreated()).toBeTruthy();

        // Verify that ensureJvm is idempotent
        java.ensureJvm(function (err) {
          expect(err).toBeFalsy();
          resolve();
        });
      });
    });
  });

  test('api', () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    expect(arrayList).toBeTruthy();
    expect(java.instanceOf(arrayList, "java.util.ArrayList")).toBeTruthy();

    expect(typeof arrayList.addSync !== 'undefined', 'Expected `addSync` to be present, but it is NOT.').toBeTruthy();
    expect(typeof arrayList.add !== 'undefined', 'Expected `add` to be present, but it is NOT.').toBeTruthy();
    expect(typeof arrayList.addPromise === 'undefined', 'Expected `addPromise` to NOT be present, but it is.').toBeTruthy();
  });

  test('importClass', () => {
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    const ArrayList = java.import("java.util.ArrayList");
    expect(ArrayList).toBeTruthy();
    const arrayList = new ArrayList();
    expect(arrayList).toBeTruthy();
    expect(arrayList.sizeSync()).toBe(0);
  });

  test('staticAPI', () => {
    const String = java.import("java.lang.String");
    expect(String).toBeTruthy();

    const api = Object.keys(String).filter((key) => typeof String[key] === 'function');
    expect(api.includes('format'), 'Expected `format` to be present, but it is NOT.').toBeTruthy();
    expect(api.includes('formatSync'), 'Expected `formatSync` to be present, but it is NOT.').toBeTruthy();
    expect(api.includes('formatAsync'), 'Expected `formatAsync` to NOT be present, but it is.').toBeFalsy();
    expect(api.includes('formatPromise'), 'Expected `formatPromise` to NOT be present, but it is.').toBeFalsy();
    expect(api.includes('formatundefined'), 'Expected `formatundefined` to NOT be present, but it is.').toBeFalsy();
  });

  test('syncCalls', () => {
    const arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.addSync("hello");
    arrayList.addSync("world");
    expect(arrayList.sizeSync()).toBe(2);
  });

  test('staticSyncCalls', () => {
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    // Among other things, java.import creates Sync functions for static methods.
    const String = java.import("java.lang.String");
    expect(String.formatSync('%s--%s', "hello", "world")).toBe("hello--world");
  });

  test('asyncCalls', async () => {
    await new Promise(resolve => {
      const arrayList = java.newInstanceSync("java.util.ArrayList");
      arrayList.add("hello", function (err, result) {
        expect(err).toBeFalsy();
        arrayList.add("world", function (err, result) {
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
});
