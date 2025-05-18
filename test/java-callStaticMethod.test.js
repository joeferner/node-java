import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

const java = getJava();

describe("Java - Call Static Method", () => {
  test("callStaticMethod", async () => {
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "staticMethod", (err, result) => {
        expect(err).toBeFalsy();
        expect(result).toBeTruthy();
        expect(result).toBe("staticMethod called");
        resolve();
      });
    });
  });

  test("callStaticMethod without a callback", () => {
    const result = java.callStaticMethod("Test", "staticMethod");
    expect(result).toBe(
      `"Static method 'staticMethod' called without a callback did you mean to use the Sync version?"`
    );
  });

  test("callStaticMethodSync", () => {
    const result = java.callStaticMethodSync("Test", "staticMethod");
    expect(result).toBeTruthy();
    expect(result).toBe("staticMethod called");
  });

  test("callStaticMethod with args", async () => {
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "staticMethod", 42, (err, result) => {
        expect(err).toBeFalsy();
        expect(result).toBeTruthy();
        expect(result).toBe(43);
        resolve();
      });
    });
  });

  test("callStaticMethodSync with args", () => {
    const result = java.callStaticMethodSync("Test", "staticMethod", 42);
    expect(result).toBeTruthy();
    expect(result).toBe(43);
  });

  test("callStaticMethodSync with BigDecimal", () => {
    const bigDecimal = java.newInstanceSync("java.math.BigDecimal", 100.1);
    const result = java.callStaticMethodSync("Test", "staticBigDecimalToString", bigDecimal);
    expect(result).toBeTruthy();
    expect(Math.abs(parseFloat(result) - 100.1) < 0.0001).toBeTruthy();
  });

  test("callStaticMethod bad class name", async () => {
    await new Promise((resolve) => {
      java.callStaticMethod("BadClassName", "staticMethod", function (err, result) {
        expect(err).toBeTruthy();
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("callStaticMethodSync bad class name", () => {
    expect(() => {
      java.callStaticMethodSync("BadClassName", "staticMethod");
    }).toThrow();
  });

  test("callStaticMethod bad arg types", async () => {
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "staticMethod", "z", function (err, result) {
        expect(err).toBeTruthy();
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("callStaticMethodSync bad arg types", () => {
    expect(() => {
      java.callStaticMethodSync("Test", "staticMethod", "z");
    }).toThrow();
  });

  test("callStaticMethod bad number of args", async () => {
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "staticMethod", 42, "z", function (err, result) {
        expect(err).toBeTruthy();
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("callStaticMethodSync bad number of args", () => {
    expect(() => {
      java.callStaticMethodSync("Test", "staticMethod", 42, "z");
    }).toThrow();
  });

  test("callStaticMethod bad method name", async () => {
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "badMethodName", function (err, result) {
        expect(err).toBeTruthy();
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("callStaticMethodSync bad method name", () => {
    expect(() => {
      java.callStaticMethodSync("Test", "badMethodName");
    }).toThrow();
  });

  test("callStaticMethod exception thrown from method (sync)", () => {
    const ex = java.newInstanceSync("java.lang.Exception", "my exception");
    let result;
    try {
      result = java.callStaticMethodSync("Test", "staticMethodThrows", ex);
      throw new Error("expected error");
    } catch (err) {
      expect(err.cause.getMessageSync()).toBe("my exception");
      expect(err.toString()).toMatch(/my exception/);
      expect(result).toBeFalsy();
    }
  });

  test("staticMethodThrows exception thrown from method (sync)", () => {
    const ex = java.newInstanceSync("java.lang.Exception", "my exception");
    try {
      java.callStaticMethodSync("Test", "staticMethodThrows", ex);
      throw new Error("should throw");
    } catch (err) {
      expect(err.toString()).toMatch(/my exception/);
    }
  });

  test("staticMethodThrows exception thrown from method", async () => {
    const ex = java.newInstanceSync("java.lang.Exception", "my exception");
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "staticMethodThrows", ex, function (err, result) {
        expect(err).toBeTruthy();
        expect(err.cause.getMessageSync()).toBe("my exception");
        expect(err.toString()).toMatch(/my exception/);
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("staticMethodThrowsNewException exception thrown from method (sync)", () => {
    try {
      java.callStaticMethodSync("Test", "staticMethodThrowsNewException");
      throw new Error("should throw");
    } catch (err) {
      expect(err.toString()).toMatch(/my exception/);
    }
  });

  test("staticMethodThrowsNewException exception thrown from method", async () => {
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "staticMethodThrowsNewException", function (err, result) {
        expect(err).toBeTruthy();
        expect(err.cause.getMessageSync()).toBe("my exception");
        expect(err.toString()).toMatch(/my exception/);
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("methodThrows exception thrown from method (sync)", () => {
    const ex = java.newInstanceSync("java.lang.Exception", "my exception");
    const myTest = java.newInstanceSync("Test");
    try {
      myTest.methodThrowsSync(ex);
      throw new Error("should throw");
    } catch (err) {
      expect(err.toString()).toMatch(/my exception/);
    }
  });

  test("methodThrows exception thrown from method", async () => {
    const ex = java.newInstanceSync("java.lang.Exception", "my exception");
    const myTest = java.newInstanceSync("Test");
    await new Promise((resolve) => {
      return myTest.methodThrows(ex, function (err) {
        expect(err.toString()).toMatch(/my exception/);
        resolve();
      });
    });
  });

  test("methodThrowsNewException exception thrown from method (sync)", () => {
    const myTest = java.newInstanceSync("Test");
    try {
      myTest.methodThrowsNewExceptionSync();
      throw new Error("should throw");
    } catch (err) {
      expect(err.toString()).toMatch(/my exception/);
    }
  });

  test("methodThrowsNewException exception thrown from method", async () => {
    const myTest = java.newInstanceSync("Test");
    await new Promise((resolve) => {
      myTest.methodThrowsNewException((err) => {
        expect(err.toString()).toMatch(/my exception/);
        resolve();
      });
    });
  });

  test("char array", async () => {
    const charArray = java.newArray("char", "hello world\n".split(""));
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "staticMethodCharArrayToString", charArray, function (err, result) {
        expect(err).toBeFalsy();
        expect(result).toBeTruthy();
        expect(result).toBe("hello world\n");
        resolve();
      });
    });
  });

  test("String passed in for Object", async () => {
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "static2Objects", "1", "2", function (err, result) {
        expect(err).toBeFalsy();
        expect(result).toBe(false);
        resolve();
      });
    });
  });

  test("java.lang.Long addition", () => {
    const javaLong = java.newInstanceSync("java.lang.Long", 5);
    expect(javaLong.toString()).toBe("5");
    const result = javaLong + 1;
    expect(result).toBe(6);
  });

  test("java.lang.Long calls (java Long)", async () => {
    const javaLong = java.newInstanceSync("java.lang.Long", 5);
    await new Promise((resolve) => {
      java.callStaticMethod("Test", "staticMethodLongToString", javaLong, function (err, result) {
        expect(err).toBeFalsy();
        expect(result).toBeTruthy();
        expect(result).toBe("5");
        resolve();
      });
    });
  });

  test("Call method that returns a long", () => {
    java.callStaticMethod("Test", "staticMethodReturnLong", function (err, result) {
      expect(err).toBeFalsy();
      expect(result).toBeTruthy();
      expect(result.longValue).toBe("9223372036854775807");
    });
  });

  test("Call method with nested enum value", () => {
    const Test = java.import("Test");
    Test.staticEnumToStringSync(Test.StaticEnum.Value1);
    const str = Test.staticEnumToStringSync(Test.StaticEnum.Value1); // call it twice to ensure memo-ize is working
    expect(str).toBe("Value1");
  });

  test("Call static method with varargs", () => {
    const Test = java.import("Test");

    let str = Test.staticVarargsSync(5, java.newArray("java.lang.String", ["a", "b", "c"]));
    expect(str).toBe("5abc");

    str = Test.staticVarargsSync(5, "a", "b", "c");
    expect(str).toBe("5abc");
  });

  test("Call static method named name_", async () => {
    await new Promise((resolve) => {
      const Test = java.import("Test");
      Test.name_((err) => {
        expect(err).toBeFalsy();
        resolve();
      });
    });
  });
});
