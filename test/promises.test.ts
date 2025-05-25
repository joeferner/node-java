import { beforeAll, describe, expect, test } from "vitest";
import { Java } from "../java";
import { getJava } from "../testHelpers";

describe("Promises", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava();
  });

  test("create an instance of a class and call methods (getClassPromise & getNamePromise)", async () => {
    // Adapted from a test in simple-test.js
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    const list = await (java as any).newInstancePromise("java.util.ArrayList");
    expect(list).toBeTruthy();

    const clazz = await list.getClassPromise();
    expect(clazz).toBeTruthy();

    const name = await clazz.getNamePromise();
    expect(name).toBe("java.util.ArrayList");
  });

  test("import and execute promisified static method", async () => {
    const Test = java.import("Test");
    const result = await Test.staticMethodPromise(99);
    expect(result).toBe(100);
  });

  test("run promisified method of Java module (newInstancePromise)", async () => {
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    const list = await (java as any).newInstancePromise("java.util.ArrayList");
    expect(list).toBeTruthy();

    const clazz = await list.getClassPromise();
    expect(clazz).toBeTruthy();

    const name = await clazz.getNamePromise();
    expect(name).toBe("java.util.ArrayList");
  });

  test("run chained promisified methods (of class java.util.ArrayList)", async () => {
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    const list = await (java as any).newInstancePromise("java.util.ArrayList");
    expect(list).toBeTruthy();

    const clazz = await list.getClassPromise();
    expect(clazz).toBeTruthy();

    const name = await clazz.getNamePromise();
    expect(name).toBe("java.util.ArrayList");

    await list.addPromise("hello");
    await list.addPromise("world");
    await list.addPromise("boo");
    const it = await list.iteratorPromise();
    expect(it).toBeTruthy();

    let val = await it.nextPromise();
    expect(val).toBeTruthy();
    console.log(typeof val, val);
    expect(val).toBe("hello"); // java.lang.InternalError exception thrown here with OpenJDK

    val = await it.nextPromise();
    expect(val).toBeTruthy();
    console.log(typeof val, val);
    expect(val).toBe("world");

    val = await it.nextPromise();
    expect(val).toBeTruthy();
    console.log(typeof val, val);
    expect(val).toBe("boo");

    const more = await it.hasNextPromise();
    console.log(typeof more, more);
    expect(more).toBeFalsy();

    await expect(async () => await it.nextPromise()).rejects.toThrowError();
  });
});
