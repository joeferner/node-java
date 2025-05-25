import { afterEach, beforeAll, describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";
import { Java } from "../java";

describe("Import Class", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava();
  });

  afterEach(() => {
    java.setStaticFieldValue("Test", "staticFieldInt", 42);
  });

  test("import", async () => {
    const Test = java.import("Test");
    expect(Test.staticFieldInt).toBe(42);
    Test.staticFieldInt = 200;
    expect(Test.staticFieldInt).toBe(200);

    expect(Test.staticMethodSync(99)).toBe(100);
    await new Promise<void>((resolve) => {
      Test.staticMethod(99, (err: Error | undefined, result: number | undefined) => {
        expect(err).toBeFalsy();
        expect(result).toBe(100);

        const testObj = new Test(5);
        expect(testObj.getIntSync()).toBe(5);
        resolve();
      });
    });
  });

  test("import TestEnum with unsable name", () => {
    const TestEnum = java.import("Test$Enum");

    // 'foo' and 'bar' are valid enum names
    expect(TestEnum.foo.toStringSync()).toBe("foo");
    expect(TestEnum.bar.toStringSync()).toBe("bar");

    ["name", "arguments", "caller"].forEach(function (prop) {
      expect(() => {
        // The enum also defines 'name', 'caller', and 'attributes', but Javascript prevents us from using them,
        // since these are unwritable properties of Function.
        TestEnum[prop].toStringSync();
      }).toThrow(TypeError);
    });
  });

  test("import TestEnum and use alternate name", () => {
    const TestEnum = java.import("Test$Enum");

    // 'foo' and 'bar' are valid enum names
    expect(TestEnum.foo.toStringSync()).toBe("foo");
    expect(TestEnum.bar.toStringSync()).toBe("bar");

    // 'name', 'caller', and 'arguments' are not, so we must use e.g. 'name_' to reference the enum.
    // But note that the value is still e.g. "name".
    expect(TestEnum.name_.toStringSync()).toBe("name");
    expect(TestEnum.arguments_.toStringSync()).toBe("arguments");
    expect(TestEnum.caller_.toStringSync()).toBe("caller");
  });
});
