import { describe, expect, test } from "vitest";
import when from 'when';
import { java } from "../testHelpers";

describe('clientPBeforeThrows', () => {
  test('clientPBeforeThrows', async () => {
    await new Promise(resolve => {
      expect(java.isJvmCreated()).toBeFalsy();

      java.asyncOptions = {
        syncSuffix: "Sync",
        promiseSuffix: 'Promise',
        promisify: require('when/node').lift // https://github.com/cujojs/when
      };

      function beforeP() {
        var promise = when.promise(function (resolve, reject) {
          expect(java.isJvmCreated()).toBeFalsy();
          throw new Error('dummy error');
        });
        return promise;
      }

      java.registerClientP(beforeP);

      java.ensureJvm().done(
        () => {
          throw new Error('expected error');
        },
        (err) => {
          expect(err && typeof err === 'object').toBeTruthy();
          expect(err).instanceOf(Error);
          expect(err.message).toBe('dummy error');
          expect(java.isJvmCreated()).toBeFalsy();
          resolve();
        }
      );
    });
  });
});
