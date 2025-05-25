// For any function, the property 'name' is an unwritable property.
// The value returned by java.import(<classname>) is a constructor-like function that has the shape of the class.
// In particular, any static methods of the class will be added as properties of the function.
// If a class has a static method named 'name', then an exception woudld be thrown when
// node-java attempts to set assign the static method to the .name property of constructor-like function.
// As a workaround, node-java will append the `ifReadOnlySuffix` to the property name.

import { beforeAll, describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";
import { Java } from "../java";

describe("unusableMethodName", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava(
      {
        syncSuffix: "Sync",
        asyncSuffix: "",
        ifReadOnlySuffix: "_alt",
      },
      {
        beforeInit: (java) => {
          function before(callback: () => void): void {
            java.classpath.push("test/");
            expect(java.isJvmCreated()).toBeFalsy();
            callback();
          }

          function after(callback: () => void): void {
            expect(java.isJvmCreated()).toBeTruthy();
            callback();
          }

          java.registerClient(before, after);
        },
      }
    );

    await new Promise<void>((resolve) => {
      java.ensureJvm(function (err: Error | undefined) {
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
      Test.name((_err: Error | undefined) => {
        throw new Error("should not get here");
      });
    }).toThrowError(TypeError);
  });

  test("unusableMethodName_callerThrows", () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    expect(() => {
      Test.caller((_err: Error | undefined) => {
        throw new Error("should not get here");
      });
    }).toThrowError(TypeError);
  });

  test("unusableMethodName_argumentsThrows", () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    expect(() => {
      Test.arguments((_err: Error | undefined) => {
        throw new Error("should not get here");
      });
    }).toThrowError(TypeError);
  });

  test("alternateMethodName_name_altWorks", async () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    await new Promise<void>((resolve) => {
      Test.name_alt((err: Error | undefined, val: string | undefined) => {
        expect(err).toBeFalsy();
        expect(val).toBe("name");
        resolve();
      });
    });
  });

  test("alternateMethodName_caller_altWorks", async () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    await new Promise<void>((resolve) => {
      Test.caller_alt((err: Error | undefined, val: string | undefined) => {
        expect(err).toBeFalsy();
        expect(val).toBe("caller");
        resolve();
      });
    });
  });

  test("alternateMethodName_arguments_altWorks", async () => {
    const Test = java.import("Test");
    expect(Test).toBeTruthy();
    await new Promise<void>((resolve) => {
      Test.arguments_alt((err: Error | undefined, val: string | undefined) => {
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
