import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

describe("clientBeforeSyncThrows", () => {
  test("clientBeforeSyncThrows", async () => {
    await getJava(
      {
        syncSuffix: "Sync",
      },
      {
        beforeInit: async (java) => {
          expect(java.isJvmCreated()).toBeFalsy();

          function before(): void {
            expect(java.isJvmCreated()).toBeFalsy();
            throw new Error("dummy error");
          }
          java.registerClient(before);

          await new Promise<void>((resolve) => {
            java.ensureJvm((err: Error | undefined) => {
              expect(err && typeof err === "object").toBeTruthy();
              expect(err).instanceOf(Error);
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
