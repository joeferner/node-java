// Just Sync and Promise, both with a non-empty suffix.

import { beforeAll, describe, expect, test } from "vitest";
import when from 'when';
import { java } from "../testHelpers";

describe('noAsync', () => {
  beforeAll(async () => {
    var api = Object.keys(java).filter((key) => typeof java[key] === 'function');
    expect(api.includes('isJvmCreated'), 'Expected `isJvmCreated` to be present, but it is NOT.').toBeTruthy();
    expect(java.isJvmCreated()).toBeFalsy();

    function before() {
      var promise = when.promise(function (resolve) {
        expect(java.isJvmCreated()).toBeFalsy();
        resolve();
      });
      return promise;
    }

    function after() {
      var promise = when.promise(function (resolve) {
        expect(java.isJvmCreated()).toBeTruthy();
        resolve();
      });
      return promise;
    }

    java.asyncOptions = {
      syncSuffix: "Sync",
      promiseSuffix: 'Promise',
      promisify: require('when/node').lift
    };
    java.registerClientP(before, after);
    java.registerClientP(null, after);
    java.registerClientP(before);

    await new Promise(resolve => {
      java.ensureJvm().done(function () {
        expect(java.isJvmCreated()).toBeTruthy();
        resolve();
      });
    });
  });

  test('api', () => {
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    expect(arrayList).toBeTruthy();
    expect(java.instanceOf(arrayList, "java.util.ArrayList")).toBeTruthy();

    expect(typeof arrayList.addSync !== 'undefined', 'Expected `addSync` to be present, but it is NOT.').toBeTruthy();
    expect(typeof arrayList.addPromise !== 'undefined', 'Expected `addPromise` to be present, but it is NOT.').toBeTruthy();
    expect(typeof arrayList.add === 'undefined', 'Expected `add` to NOT be present, but it is.').toBeTruthy();
    expect(typeof arrayList.addAsync === 'undefined', 'Expected `addAsync` to NOT be present, but it is.').toBeTruthy();
  });

  test('importClass', () => {
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    var ArrayList = java.import("java.util.ArrayList");
    expect(ArrayList).toBeTruthy();
    var arrayList = new ArrayList();
    expect(arrayList).toBeTruthy();
    expect(arrayList.sizeSync()).toBe(0);
  });

  test('staticAPI', () => {
    var String = java.import("java.lang.String");
    expect(String).toBeTruthy();

    var api = Object.keys(String).filter((key) => typeof String[key] === 'function');
    expect(api.includes('formatSync'), 'Expected `formatSync` to be present, but it is NOT.').toBeTruthy();
    expect(api.includes('formatPromise'), 'Expected `formatPromise` to be present, but it is NOT.').toBeTruthy();
    expect(api.includes('format'), 'Expected `format` to NOT be present, but it is.').toBeFalsy();
    expect(api.includes('formatAsync'), 'Expected `formatAsync` to NOT be present, but it is.').toBeFalsy();
    expect(api.includes('formatundefined'), 'Expected `formatundefined` to NOT be present, but it is.').toBeFalsy();
  });

  test('syncCalls', () => {
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    arrayList.addSync("hello");
    arrayList.addSync("world");
    expect(arrayList.sizeSync()).toBe(2);
  });

  test('sStaticSyncCalls', () => {
    // Note: java.import executes javascript code in lib/nodeJavaBridge that makes sync calls to java classes.
    // Among other things, java.import creates Sync functions for static methods.
    var String = java.import("java.lang.String");
    expect(String.formatSync('%s--%s', "hello", "world")).toBe("hello--world");
  });

  test('promiseCalls', async () => {
    var arrayList = java.newInstanceSync("java.util.ArrayList");
    await new Promise(resolve => {
      arrayList.addPromise("hello")
        .then(() => { return arrayList.addPromise("world"); })
        .then(() => { return arrayList.sizePromise(); })
        .then((size) => {
          expect(size).toBe(2);
          resolve();
        });
    });
  });
});