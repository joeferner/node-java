import { afterEach, beforeAll, describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";
import { Java } from "../java";

describe("Java - Static Field", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava();
  });

  afterEach(() => {
    java.setStaticFieldValue("Test", "staticFieldInt", 42);
  });

  test("getStaticFieldValue int", () => {
    const val = java.getStaticFieldValue("Test", "staticFieldInt");
    expect(val).toBe(42);
  });

  test("setStaticFieldValue int", () => {
    java.setStaticFieldValue("Test", "staticFieldInt", 112);
    const val = java.getStaticFieldValue("Test", "staticFieldInt");
    expect(val).toBe(112);
  });

  test("getStaticFieldValue double", () => {
    const val = java.getStaticFieldValue("Test", "staticFieldDouble");
    expect(val).toBe(42.5);
  });

  test("setStaticFieldValue double", () => {
    java.setStaticFieldValue("Test", "staticFieldDouble", 112.12);
    const val = java.getStaticFieldValue("Test", "staticFieldDouble");
    expect(val).toBe(112.12);
  });

  test("setStaticFieldValue double (set int)", () => {
    java.setStaticFieldValue("Test", "staticFieldDouble", 112);
    const val = java.getStaticFieldValue("Test", "staticFieldDouble");
    expect(val).toBe(112);
  });
});
