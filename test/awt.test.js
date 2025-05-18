import path from "node:path";
import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

const java = getJava();

describe("AWT", () => {
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
