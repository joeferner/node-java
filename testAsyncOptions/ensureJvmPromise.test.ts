import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

describe("ensureJvmPromise", () => {
  test("calling ensureJvm as a promise", async () => {
    await getJava(
      {
        syncSuffix: "Sync",
        asyncSuffix: "",
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
