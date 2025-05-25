import { beforeEach, describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";
import { Java, JavaObject } from "../java";

describe("Java Object", () => {
  let testObj: JavaObject;
  let java!: Java;

  beforeEach(async () => {
    java = await getJava();
    testObj = java.newInstanceSync("Test");
  });

  test("field", () => {
    expect(testObj.nonstaticInt).toBe(42);
    testObj.nonstaticInt = 112;
    expect(testObj.nonstaticInt).toBe(112);
  });
});
