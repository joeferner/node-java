import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

describe("clientBeforeError", () => {
  test("clientBeforeError", async () => {
    await getJava(
      {
        syncSuffix: "Sync",
      },
      {
        beforeInit: async (java) => {
          expect(java.isJvmCreated()).toBeFalsy();

          function before(callback: (err: Error) => void): void {
            expect(java.isJvmCreated()).toBeFalsy();
            callback(new Error("dummy error"));
          }

          java.registerClient(before);

          await new Promise<void>((resolve) => {
            java.ensureJvm((err) => {
              expect(err).toBeTruthy();
              expect(typeof err).toBe("object");
              expect(err).toBeInstanceOf(Error);
              expect(err?.message).toBe("dummy error");
              expect(java.isJvmCreated()).toBeFalsy();
              resolve();
            });
          });
        },
      }
    );
  });
});
