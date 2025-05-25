import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

describe("invalidLaunch", () => {
  test("calling ensureJvm as a promise when no promisify function supplied", async () => {
    await getJava(
      {
        syncSuffix: "Sync",
        asyncSuffix: "",
      },
      {
        beforeInit: (java) => {
          expect(java.isJvmCreated()).toBeFalsy();

          // First show that if asyncOptions.promisify is undefined, using the promise variant of ensureJvm throws an error.
          expect(() => {
            void java.ensureJvm();
          }).toThrow(/requires its one argument to be a callback function/);

          expect(java.isJvmCreated()).toBeFalsy();
        },
      }
    );
  });

  test("callbackNotAFunction", async () => {
    await getJava(
      {
        syncSuffix: "",
        promiseSuffix: "P",
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        promisify: ((await import("when/node")) as any).lift, // https://github.com/cujojs/when
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
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        promisify: ((await import("when/node")) as any).lift, // https://github.com/cujojs/when
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
