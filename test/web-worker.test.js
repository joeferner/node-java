
import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers.js";
import { Worker } from "node:worker_threads";
import path from "node:path";
import findRoot from 'find-root';

const java = getJava();
const root = findRoot(__dirname);

describe('web-worker', () => {
    test('run java in a web worker', async () => {
        await new Promise((resolve, reject) => {
            const version = java.callStaticMethodSync("java.lang.System", "getProperty", "java.version");
            expect(version).toBeTruthy();

            const worker = new Worker(path.join(root, 'test/web-worker.worker.js'));
            worker.on('message', (result) => {
                expect(result).toBe(42);
                resolve();
            });
            worker.on('error', reject);
            worker.on('messageerror', () => reject(new Error('message error')));
            worker.on('exit', () => reject(new Error('exit')));

            worker.postMessage({ name: 'getJavaVersion' });
        });
    });
});
