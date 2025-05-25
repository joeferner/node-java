import { beforeAll, describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";
import { Java } from "../java";

describe("instanceOf", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava();
  });

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
