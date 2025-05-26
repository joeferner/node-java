import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

describe("invalidLaunch", () => {
  test("callbackNotAFunction", async () => {
    await getJava(
      {
        syncSuffix: "",
        promiseSuffix: "P",
      },
      {
        beforeInit: (java) => {
          expect(java.isJvmCreated()).toBeFalsy();

          expect(() => {
            // eslint-disable-next-line @typescript-eslint/no-explicit-any
            (java as any).ensureJvm("foo");
          }).toThrow(/requires its one argument to be a callback function/);

          expect(java.isJvmCreated()).toBeFalsy();
        },
      }
    );
  });

  test("jvmCanStillBeLaunched", async () => {
    await getJava(
      {
        syncSuffix: "",
        promiseSuffix: "P",
      },
      {
        beforeInit: async (java) => {
          expect(java.isJvmCreated()).toBeFalsy();

          await java.ensureJvm();
          expect(java.isJvmCreated()).toBeTruthy();
        },
      }
    );
  });
});
