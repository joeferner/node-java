import { describe, expect, test } from "vitest";
import when from "when";
import { java } from "../testHelpers";

describe("clientPBeforeError", () => {
  test("clientPBeforeError", async () => {
    await new Promise((resolve) => {
      expect(java.isJvmCreated()).toBeFalsy();

      java.asyncOptions = {
        syncSuffix: "Sync",
        promiseSuffix: "Promise",
        promisify: require("when/node").lift, // https://github.com/cujojs/when
      };

      function beforeP() {
        const promise = when.promise((_resolve, reject) => {
          expect(java.isJvmCreated()).toBeFalsy();
          reject(new Error("dummy error"));
        });
        return promise;
      }

      java.registerClientP(beforeP);

      java.ensureJvm().done(
        () => {
          throw new Error("expect error");
        },
        (err) => {
          expect(err && typeof err === "object").toBeTruthy();
          expect(err).instanceOf(Error);
          expect(err.message).toBe("dummy error");
          expect(java.isJvmCreated()).toBeFalsy();
          resolve();
        }
      );
    });
  });
});
