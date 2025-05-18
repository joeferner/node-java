import { describe, expect, test } from "vitest";
import { java } from "../testHelpers";

describe("invalidLaunch", () => {
  test("failedLaunch", () => {
    expect(java.isJvmCreated()).toBeFalsy();

    java.asyncOptions = {
      syncSuffix: "Sync",
      asyncSuffix: "",
    };

    // First show that if asyncOptions.promisify is undefined, using the promise variant of ensureJvm throws an error.
    expect(() => {
      java.ensureJvm();
    }).toThrow(/requires its one argument to be a callback function/);

    expect(java.isJvmCreated()).toBeFalsy();
  });

  test("callbackNotAFunction", () => {
    expect(java.isJvmCreated()).toBeFalsy();

    java.asyncOptions = {
      syncSuffix: "",
      promiseSuffix: "P",
      promisify: require("when/node").lift, // https://github.com/cujojs/when
    };

    expect(() => {
      java.ensureJvm("foo");
    }).toThrow(/requires its one argument to be a callback function/);

    expect(java.isJvmCreated()).toBeFalsy();
  });

  test("jvmCanStillBeLaunched", async () => {
    // None of the previous tests should have caused the JVM to be created, so it should still be possible to create one.

    expect(java.isJvmCreated()).toBeFalsy();

    java.asyncOptions = {
      syncSuffix: "",
      promiseSuffix: "P",
      promisify: require("when/node").lift, // https://github.com/cujojs/when
    };

    await new Promise((resolve) => {
      java.ensureJvm().done(function () {
        expect(java.isJvmCreated()).toBeTruthy();
        resolve();
      });
    });
  });
});
