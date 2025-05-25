import path from "node:path";
import { beforeAll, describe, expect, test } from "vitest";
import { Java } from "../java";
import { getJava } from "../testHelpers";

describe("AWT", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava();
  });

  test("calling AWT method (see issue 1)", () => {
    const headlessVal = java.callStaticMethodSync("java.lang.System", "getProperty", "java.awt.headless");
    console.log("java.awt.headless =", headlessVal);
    expect(headlessVal).toBe("true");
    const filename = path.join(path.dirname(__filename), "nodejs.png");
    const file = java.newInstanceSync("java.io.File", filename);
    const image = java.callStaticMethodSync("javax.imageio.ImageIO", "read", file);
    expect(image).toBeTruthy();
  });
});
