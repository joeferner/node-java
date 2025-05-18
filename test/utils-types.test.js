import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

const java = getJava();

describe("Utils - Types", () => {
  test("Array of Objects", () => {
    let val = java.getStaticFieldValue("Test", "staticArrayObjects");
    expect(val).toBeNull();

    java.setStaticFieldValue(
      "Test",
      "staticArrayObjects",
      java.newArray("Test", [
        java.newInstanceSync("Test", 1),
        java.newInstanceSync("Test", 2),
        java.newInstanceSync("Test", 3),
      ])
    );

    val = java.getStaticFieldValue("Test", "staticArrayObjects");
    expect(val).toBeTruthy();
    expect(val.length).toBe(3);
    expect(val[0].getIntSync()).toBe(1);
    expect(val[1].getIntSync()).toBe(2);
    expect(val[2].getIntSync()).toBe(3);
  });

  test("Static Method Overload", () => {
    let result = java.callStaticMethodSync("Test", "staticMethodOverload", "a");
    expect(result).toBe(1);
    result = java.callStaticMethodSync("Test", "staticMethodOverload", 1);
    expect(result).toBe(2);
    result = java.callStaticMethodSync("Test", "staticMethodOverload", java.newInstanceSync("Test$SuperClass"));
    expect(result).toBe(3);
    result = java.callStaticMethodSync("Test", "staticMethodOverload", java.newInstanceSync("Test$SubClass"));
    expect(result).toBe(4);
  });

  test("Method Overload", () => {
    const testObj = java.newInstanceSync("Test");
    let result = testObj.methodOverloadSync("a");
    expect(result).toBe(1);
    result = testObj.methodOverloadSync(1);
    expect(result).toBe(2);
    result = testObj.methodOverloadSync(java.newInstanceSync("Test$SuperClass"));
    expect(result).toBe(3);
    result = testObj.methodOverloadSync(java.newInstanceSync("Test$SubClass"));
    expect(result).toBe(4);
  });

  test("Char array", () => {
    const originalArray = "hello 世界\n".split("");
    const Arrays = java.import("java.util.Arrays");
    const arr1 = java.newArray("char", originalArray);
    const list = Arrays.asListSync(arr1);
    const arr2 = list.toArraySync();
    expect(arr2.length).toBe(1);
    expect(arr2[0].length).toBe(9);
    const isTypedArrayReturn = !(typeof arr2[0][0] === "string");
    for (let i = 0; i < originalArray.length; i++) {
      if (isTypedArrayReturn) {
        expect(arr2[0][i]).toBe(originalArray[i].charCodeAt(0));
      } else {
        expect(arr2[0][i]).toBe(originalArray[i]);
      }
    }
  });
});
