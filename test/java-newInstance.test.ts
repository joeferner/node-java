import { beforeAll, describe, expect, test } from "vitest";
import { Java, JavaObject } from "../java";
import { getJava } from "../testHelpers";

describe("Java - New Instance", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava();
  });

  test("newInstance", async () => {
    await new Promise<void>((resolve) => {
      java.newInstance("Test", (err: Error | undefined, result: JavaObject | undefined) => {
        expect(err).toBeFalsy();
        expect(result).toBeTruthy();
        expect(result.getClassSync().toStringSync()).toBe("class Test");
        expect(result.getInt).toBeTruthy();
        expect(result.getIntSync).toBeTruthy();
        expect(result.staticMethod).toBeFalsy();
        expect(result.staticMethodSync).toBeFalsy();
        expect(result.nonstaticInt).toBe(42);
        resolve();
      });
    });
  });

  test("newInstanceSync", () => {
    const result = java.newInstanceSync("Test");
    expect(result).toBeTruthy();
    expect(result.getClassSync().toStringSync()).toBe("class Test");
  });

  test("newInstance with args", async () => {
    await new Promise<void>((resolve) => {
      java.newInstance("Test", 42, (err: Error | undefined, result: JavaObject | undefined) => {
        expect(err).toBeFalsy();
        expect(result).toBeTruthy();
        expect(result.getIntSync()).toBe(42);
        resolve();
      });
    });
  });

  test("newInstanceSync with args", () => {
    const result = java.newInstanceSync("Test", 42);
    expect(result).toBeTruthy();
    expect(result.getIntSync()).toBe(42);
  });

  test("newInstance bad class name", async () => {
    await new Promise<void>((resolve) => {
      java.newInstance("BadClassName", (err: Error | undefined, result: JavaObject | undefined) => {
        expect(err).toBeTruthy();
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("newInstanceSync bad class name", () => {
    expect(() => {
      java.newInstanceSync("BadClassName");
    }).toThrow();
  });

  test("newInstance bad arg types", async () => {
    await new Promise<void>((resolve) => {
      java.newInstance("Test", "a", (err: Error | undefined, result: JavaObject | undefined) => {
        expect(err).toBeTruthy();
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("newInstanceSync bad arg types", () => {
    expect(() => {
      java.newInstanceSync("Test", "a");
    }).toThrow();
  });

  test("newInstance bad number of args", async () => {
    await new Promise<void>((resolve) => {
      java.newInstance("Test", 42, 15, (err: Error | undefined, result: JavaObject | undefined) => {
        expect(err).toBeTruthy();
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("newInstanceSync bad number of args", () => {
    expect(() => {
      java.newInstanceSync("Test", 42, 15);
    }).toThrow();
  });

  test("newInstance exception thrown from constructor", async () => {
    const ex = java.newInstanceSync("java.lang.Exception", "my exception");
    await new Promise<void>((resolve) => {
      java.newInstance("TestExceptions", ex, (err: Error | undefined, result: JavaObject | undefined) => {
        expect(err).toBeTruthy();
        expect(err?.toString()).toMatch(/my exception/);
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("newInstanceSync exception thrown from constructor", () => {
    const ex = java.newInstanceSync("java.lang.Exception", "my exception");
    expect(() => java.newInstanceSync("TestExceptions", ex)).toThrowError(/my exception/);
  });

  test("newInstanceSync with varargs", () => {
    let result = java.newInstanceSync("Test", 42, java.newArray("java.lang.String", ["a", "b"]));
    expect(result).toBeTruthy();

    result = java.newInstanceSync("Test", 42, "a");
    expect(result).toBeTruthy();

    result = java.newInstanceSync("Test", 42, "a", "b", "c");
    expect(result).toBeTruthy();
  });
});
