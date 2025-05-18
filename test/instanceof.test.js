import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

const java = getJava();

describe("instanceOf", () => {
  test("working", () => {
    const subclass = java.newInstanceSync("Test$SubClass");
    if (!java.instanceOf(subclass, "Test$SuperClass")) {
      throw new Error(subclass.getNameSync() + " should be an instance of Test$SuperClass");
    }
  });

  test("non-java object", () => {
    if (java.instanceOf({}, "Test$SuperClass")) {
      throw new Error("javascript objects are not instances of anything");
    }
  });

  test("bad type", () => {
    const subclass = java.newInstanceSync("Test$SubClass");
    expect(() => {
      java.instanceOf(subclass, "BadClassName");
    }).toThrow();
  });
});
