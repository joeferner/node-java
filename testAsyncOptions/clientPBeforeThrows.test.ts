import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

describe("clientPBeforeThrows", () => {
  test("clientPBeforeThrows", async () => {
    await getJava(
      {
        syncSuffix: "Sync",
        promiseSuffix: "Promise",
      },
      {
        beforeInit: async (java) => {
          expect(java.isJvmCreated()).toBeFalsy();

          function beforeP(): Promise<void> {
            return new Promise(() => {
              expect(java.isJvmCreated()).toBeFalsy();
              throw new Error("dummy error");
            });
          }

          java.registerClientP(beforeP);

          await new Promise<void>((resolve) => {
            java.ensureJvm().then(
              () => {
                throw new Error("expected error");
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
