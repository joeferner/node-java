import { beforeEach, describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

const java = getJava();

describe('Java Object', () => {
  let testObj;

  beforeEach(() => {
    testObj = java.newInstanceSync("Test");
  })

  test("field", () => {
    expect(testObj.nonstaticInt).toBe(42);
    testObj.nonstaticInt = 112;
    expect(testObj.nonstaticInt).toBe(112);
  });
});
