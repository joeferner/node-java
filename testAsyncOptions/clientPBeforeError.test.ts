import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

describe("clientPBeforeError", () => {
  test("clientPBeforeError", async () => {
    await getJava(
      {
        syncSuffix: "Sync",
        promiseSuffix: "Promise",
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        promisify: ((await import("when/node")) as any).lift, // https://github.com/cujojs/when
      },
      {
        beforeInit: async (java) => {
          expect(java.isJvmCreated()).toBeFalsy();

          function beforeP(): Promise<void> {
            return new Promise((_resolve, reject) => {
              expect(java.isJvmCreated()).toBeFalsy();
              reject(new Error("dummy error"));
            });
          }
          java.registerClientP(beforeP);

          await new Promise<void>((resolve) => {
            java.ensureJvm().then(
              () => {
                throw new Error("expect error");
              },
              (err: Error | undefined) => {
                expect(err && typeof err === "object").toBeTruthy();
                expect(err).instanceOf(Error);
                expect(err?.message).toBe("dummy error");
                expect(java.isJvmCreated()).toBeFalsy();
                resolve();
              }
            );
          });
        },
      }
    );
  });
});
