import { afterEach, describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

const java = getJava();

describe("Java - Static Field", () => {
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
