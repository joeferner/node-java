import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

const java = getJava();

describe("varargs", () => {
  test("array signature inference", () => {
    const Test = java.import("Test");
    expect(Test.varArgsSignatureSync([])).toBe("Object...");
    expect(Test.varArgsSignatureSync(["a", "b"])).toBe("String...");
    expect(Test.varArgsSignatureSync([1, 2])).toBe("Integer...");
    expect(Test.varArgsSignatureSync([1.1, 2])).toBe("Number...");
    expect(Test.varArgsSignatureSync([1.1, "a"])).toBe("Object...");
    expect(Test.varArgsSignatureSync([true, "a"])).toBe("Object...");
    expect(Test.varArgsSignatureSync([true, 1])).toBe("Object...");
    expect(Test.varArgsSignatureSync([true, 1.1])).toBe("Object...");
    expect(Test.varArgsSignatureSync([true, false])).toBe("Boolean...");
  });

  test("variadic signature inference", () => {
    const Test = java.import("Test");
    expect(Test.varArgsSignatureSync()).toBe("Object...");
    expect(Test.varArgsSignatureSync("a", "b")).toBe("String...");
    expect(Test.varArgsSignatureSync(1, 2)).toBe("Integer...");
    expect(Test.varArgsSignatureSync(1.1, 2)).toBe("Number...");
    expect(Test.varArgsSignatureSync(1.1, "a")).toBe("Object...");
    expect(Test.varArgsSignatureSync(true, "a")).toBe("Object...");
    expect(Test.varArgsSignatureSync(true, 1)).toBe("Object...");
    expect(Test.varArgsSignatureSync(true, 1.1)).toBe("Object...");
    expect(Test.varArgsSignatureSync(true, false)).toBe("Boolean...");
  });

  test("variadic no args", () => {
    const String = java.import("java.lang.String");
    expect(String.formatSync("nothing")).toBe("nothing");
  });

  test("variadic one args", () => {
    const String = java.import("java.lang.String");
    expect(String.formatSync("%s", "hello")).toBe("hello");
  });

  test("variadic two args", () => {
    const String = java.import("java.lang.String");
    expect(String.formatSync("%s--%s", "hello", "world")).toBe("hello--world");
  });

  test("newArray(Object) no args passed", () => {
    const String = java.import("java.lang.String");
    expect(String.formatSync("nothing", java.newArray("java.lang.Object", []))).toBe("nothing");
  });

  test("newArray(Object) one args", () => {
    const String = java.import("java.lang.String");
    expect(String.formatSync("%s", java.newArray("java.lang.Object", ["hello"]))).toBe("hello");
  });

  test("newArray(Object) two args", () => {
    const String = java.import("java.lang.String");
    expect(String.formatSync("%s--%s", java.newArray("java.lang.Object", ["hello", "world"]))).toBe("hello--world");
  });

  test("Call static method with variadic varargs", () => {
    const Test = java.import("Test");
    expect(Test.staticVarargsSync(5)).toBe("5");
    expect(Test.staticVarargsSync(5, "a")).toBe("5a");
    expect(Test.staticVarargsSync(5, "a", "b")).toBe("5ab");
    expect(Test.staticVarargsSync(5, "a", "b", "c")).toBe("5abc");
  });

  test("Call static varargs method with plain array", () => {
    const Test = java.import("Test");
    expect(Test.staticVarargsSync(5, ["a"])).toBe("5a");
    expect(Test.staticVarargsSync(5, ["a", "b"])).toBe("5ab");
    expect(Test.staticVarargsSync(5, ["a", "b", "c"])).toBe("5abc");
  });

  test("Call static varags method with newArray", () => {
    const Test = java.import("Test");
    expect(Test.staticVarargsSync(5, java.newArray("java.lang.String", ["a"]))).toBe("5a");
    expect(Test.staticVarargsSync(5, java.newArray("java.lang.String", ["a", "b", "c"]))).toBe("5abc");
  });
});
