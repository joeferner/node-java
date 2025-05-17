import { describe, expect, test } from "vitest";
import { java } from "../testHelpers";

describe('clientBeforeError', () => {
  test('clientBeforeError', async () => {
    await new Promise(resolve => {
      expect(java.isJvmCreated()).toBeFalsy();

      java.asyncOptions = {
        syncSuffix: "Sync",
      };

      function before(callback) {
        expect(java.isJvmCreated()).toBeFalsy();
        callback(new Error('dummy error'));
      }

      java.registerClient(before);

      java.ensureJvm(function (err) {
        expect(err && typeof err === 'object').toBeTruthy();
        expect(err).toBeInstanceOf(Error);
        expect(err.message).toBe('dummy error');
        expect(java.isJvmCreated()).toBeFalsy();
        resolve();
      });
    });
  });
});
