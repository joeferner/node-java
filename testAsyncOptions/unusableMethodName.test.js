// For any function, the property 'name' is an unwritable property.
// The value returned by java.import(<classname>) is a constructor-like function that has the shape of the class.
// In particular, any static methods of the class will be added as properties of the function.
// If a class has a static method named 'name', then an exception woudld be thrown when
// node-java attempts to set assign the static method to the .name property of constructor-like function.
// As a workaround, node-java will append the `ifReadOnlySuffix` to the property name.

import { beforeAll, describe, expect, test } from "vitest";
import { java } from "../testHelpers";

describe("unusableMethodName", () => {
  beforeAll(async () => {
    await new Promise((resolve) => {
      function before(callback) {
        java.classpath.push("test/");
        expect(java.isJvmCreated()).toBeFalsy();
        callback();
      }

      function after(callback) {
        expect(java.isJvmCreated()).toBeTruthy();
        callback();
      }

      java.asyncOptions = {
        syncSuffix: "Sync",
        asyncSuffix: "",
        ifReadOnlySuffix: "_alt",
      };
      java.registerClient(before, after);

      java.ensureJvm(function (err) {
        expect(err).toBeFalsy();
        expect(java.isJvmCreated()).toBeTruthy();
        resolve();
      });
    });
  });

  test("unusableMethodName_nameThrows", () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    expect(() => {
      Test.name(function (_err) {
        throw new Error("should not get here");
      });
    }).toThrowError(TypeError);
  });

  test("unusableMethodName_callerThrows", () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    expect(() => {
      Test.caller(function (_err) {
        throw new Error("should not get here");
      });
    }).toThrowError(TypeError);
  });

  test("unusableMethodName_argumentsThrows", () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    expect(() => {
      Test.arguments(function (_err) {
        throw new Error("should not get here");
      });
    }).toThrowError(TypeError);
  });

  test("alternateMethodName_name_altWorks", async () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    await new Promise((resolve) => {
      Test.name_alt(function (err, val) {
        expect(err).toBeFalsy();
        expect(val).toBe("name");
        resolve();
      });
    });
  });

  test("alternateMethodName_caller_altWorks", async () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    await new Promise((resolve) => {
      Test.caller_alt(function (err, val) {
        expect(err).toBeFalsy();
        expect(val).toBe("caller");
        resolve();
      });
    });
  });

  test("alternateMethodName_arguments_altWorks", async () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    await new Promise((resolve) => {
      Test.arguments_alt(function (err, val) {
        expect(err).toBeFalsy();
        expect(val).toBe("arguments");
        resolve();
      });
    });
  });

  test("reservedFieldName", () => {
    const TestEnum = java.import("Test$Enum");
    expect(TestEnum).toBeTruthy();

    // 'foo' and 'bar' are valid enum names
    expect(TestEnum.foo.toStringSync()).toBe("foo");
    expect(TestEnum.bar.toStringSync()).toBe("bar");

    // TestEnum.name is actually the name of the proxy constructor function.
    expect(TestEnum.name).toBe("javaClassConstructorProxy");

    // Instead we need to access TestEnum.name_alt
    expect(TestEnum.name_alt.toString()).toBe("name");
    expect(TestEnum.caller_alt.toString()).toBe("caller");
    expect(TestEnum.arguments_alt.toString()).toBe("arguments");
  });
});
