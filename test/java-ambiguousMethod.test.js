import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

const java = getJava();

describe('Java - Call Ambiguous Method', () => {
  test("staticMethodAmbiguous (sync) - int passed to double", () => {
    const result = java.callStaticMethodSync('Test', 'staticMethodAmbiguous(Ljava/lang/Double;)I', 1);
    expect(result).toBe(1);
  });

  test("staticMethodAmbiguous (sync) - double passed to int", () => {
    const result = java.callStaticMethodSync('Test', 'staticMethodAmbiguous(Ljava/lang/Integer;)I', 1.1);
    expect(result).toBe(2);
  });

  test("staticMethodAmbiguous (sync) - method not found wrong argument type", () => {
    expect(() => {
      java.callStaticMethodSync('Test', 'staticMethodAmbiguous(Ljava/lang/String;)I', 1);
    }).toThrow();
  });

  test("staticMethodAmbiguous (sync) - method failed because argument count mismatch", () => {
    expect(() => {
      java.callStaticMethodSync('Test', 'staticMethodAmbiguous(Ljava/lang/Integer;)I', 1, 2);
    }).toThrow();
  });

  test("staticMethodAmbiguous - int passed to double", async () => {
    await new Promise(resolve => {
      java.callStaticMethod('Test', 'staticMethodAmbiguous(Ljava/lang/Double;)I', 1, function (err, result) {
        expect(err).toBeFalsy();
        expect(result).toBe(1);
        resolve();
      });
    });
  });

  test("staticMethodAmbiguous - double passed to int", async () => {
    await new Promise(resolve => {
      java.callStaticMethod('Test', 'staticMethodAmbiguous(Ljava/lang/Integer;)I', 1.1, function (err, result) {
        expect(err).toBeFalsy();
        expect(result).toBe(2);
        resolve();
      });
    });
  });

  test("staticMethodAmbiguous - method not found", async () => {
    await new Promise(resolve => {
      java.callStaticMethod('Test', 'staticMethodAmbiguous(Ljava/lang/String;)I', 1, function (err, result) {
        expect(err).toBeTruthy();
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("staticMethodAmbiguous - method argument count mismatch", async () => {
    await new Promise(resolve => {
      java.callStaticMethod('Test', 'staticMethodAmbiguous(Ljava/lang/Integer;)I', 1, 2, function (err, result) {
        expect(err).toBeTruthy();
        expect(result).toBeFalsy();
        resolve();
      });
    });
  });

  test("methodAmbiguous (sync) - int passed to double", () => {
    const myTest = java.newInstanceSync("Test");
    const result = java.callMethodSync(myTest, 'methodAmbiguous(Ljava/lang/Double;)I', 1);
    expect(result).toBe(1);
  });

  test("methodAmbiguous (sync) - double passed to int", () => {
    const myTest = java.newInstanceSync("Test");
    const result = java.callMethodSync(myTest, 'methodAmbiguous(Ljava/lang/Integer;)I', 1.1);
    expect(result).toBe(2);
  });

  test("methodAmbiguous (sync) - method not found wrong argument type", () => {
    const myTest = java.newInstanceSync("Test");
    expect(() => {
      java.callMethodSync(myTest, 'methodAmbiguous(Ljava/lang/String;)I', 1);
    }).toThrow();
  });

  test("methodAmbiguous (sync) - method failed because argument count mismatch", () => {
    const myTest = java.newInstanceSync("Test");
    expect(() => {
      java.callMethodSync(myTest, 'methodAmbiguous(Ljava/lang/Integer;)I', 1, 2);
    }).toThrow();
  });
});
