import { beforeAll, describe, expect, test } from "vitest";
import { Java, JavaObject } from "../java";
import { expectJavaError, getJava } from "../testHelpers";

describe("Dynamic Proxy", () => {
  let java!: Java;

  beforeAll(async () => {
    java = await getJava();
  });

  test("0 Arguments", () => {
    let callCount = 0;

    const myProxy = java.newProxy("RunInterface$Interface0Arg", {
      run: function () {
        callCount++;
      },
    });

    const runInterface = java.newInstanceSync("RunInterface");
    runInterface.run0ArgsSync(myProxy);

    expect(callCount).toBe(2);
  });

  test("1 Arguments", () => {
    let runData = "";

    const myProxy = java.newProxy("RunInterface$Interface1Arg", {
      run: (str: string) => {
        runData += str;
      },
    });

    const runInterface = java.newInstanceSync("RunInterface");
    runInterface.run1ArgsSync(myProxy);

    expect(runData).toBe("test1test1");
  });

  test("1 Arguments with return data", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {
      run: (i: number) => {
        return i + 1;
      },
    });

    const runInterface = java.newInstanceSync("RunInterface");
    const result = runInterface.runWithReturnSync(myProxy);

    expect(result).toBe(43);
  });

  test("Listener test", () => {
    let runData = "";

    const myProxy = java.newProxy("ListenerInterface", {
      onEvent: (_list: JavaObject, _runtime: JavaObject) => {
        runData = "onEvent";
      },
    });

    const listenerTester = java.newInstanceSync("ListenerTester");
    listenerTester.setListenerSync(myProxy);
    listenerTester.raiseEventSync();

    expect(runData).toBe("onEvent");
  });

  test("thread", async () => {
    await new Promise<void>((resolve, reject) => {
      let callCount = 0;

      const myProxy = java.newProxy("java.lang.Runnable", {
        run: function () {
          callCount++;
        },
      });

      const thread = java.newInstanceSync("java.lang.Thread", myProxy);
      thread.startSync();

      let timeout = 50;

      function waitForThread(): void {
        if (callCount === 1) {
          return resolve();
        }
        timeout--;
        if (timeout < 0) {
          return reject(new Error("Timeout"));
        }
        setTimeout(waitForThread, 100);
      }

      waitForThread();
    });
  });

  test("thread issue #143", async () => {
    await new Promise<void>((resolve) => {
      const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {
        run: (i: number) => {
          return i - 1;
        },
      });

      const runInterface = java.newInstanceSync("RunInterface");
      runInterface.runInAnotherThread(myProxy, (err: Error | undefined, result: number) => {
        expect(err).toBeUndefined();
        expect(result).toBe(45);
        resolve();
      });
    });
  });

  test("java equals()", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {});

    const runInterface = java.newInstanceSync("RunInterface");
    const result = runInterface.runEqualsSync(myProxy);

    expect(result).toBe(false);
  });

  test("java equals() same instance", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {});

    const runInterface = java.newInstanceSync("RunInterface");
    runInterface.setInstanceSync(myProxy);
    const result = runInterface.runEqualsInstanceSync(myProxy);

    expect(result).toBe(true);
  });

  test("java equals() different instance", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {});
    const myProxy2 = java.newProxy("RunInterface$InterfaceWithReturn", {});

    const runInterface = java.newInstanceSync("RunInterface");
    runInterface.setInstanceSync(myProxy);
    const result = runInterface.runEqualsInstanceSync(myProxy2);

    expect(result).toBe(false);
  });

  test("js equals()", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {
      equals: (_obj: JavaObject) => {
        return true;
      },
    });

    const runInterface = java.newInstanceSync("RunInterface");
    const result = runInterface.runEqualsSync(myProxy);

    expect(result).toBe(true);
  });

  test("java hashCode()", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {});

    const runInterface = java.newInstanceSync("RunInterface");
    const result = runInterface.runHashCodeSync(myProxy);
    const result2 = runInterface.runHashCodeSync(myProxy);
    const systemHashCode = java.callStaticMethodSync("java.lang.System", "identityHashCode", myProxy);

    expect(result).toBe(result2);
    expect(result).toBe(systemHashCode);
  });

  test("js hashCode()", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {
      hashCode: function () {
        return 1234;
      },
    });

    const runInterface = java.newInstanceSync("RunInterface");
    const result = runInterface.runHashCodeSync(myProxy);

    expect(result).toBe(1234);
  });

  test("java toString()", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {});

    const runInterface = java.newInstanceSync("RunInterface");
    const result = runInterface.runToStringSync(myProxy);

    expect(result).toBe("[object Object]");
  });

  test("js toString()", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {
      toString: () => {
        return "myRunInterface";
      },
    });

    const runInterface = java.newInstanceSync("RunInterface");
    const result = runInterface.runToStringSync(myProxy);

    expect(result).toBe("myRunInterface");
  });

  test("js string error", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {
      run: (_i: JavaObject) => {
        // eslint-disable-next-line @typescript-eslint/only-throw-error
        throw "myError";
      },
    });

    const runInterface = java.newInstanceSync("RunInterface");
    try {
      runInterface.runWithReturnSync(myProxy);
      throw new Error("Exception was not thrown");
    } catch (e) {
      expectJavaError(e);
      expect(e.cause.getClassSync().getNameSync()).toBe("java.lang.RuntimeException");
      expect(e.message).toMatch(/Caused by: node\.NodeJsException:.*myError/);
    }
  });

  test("js Error", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {
      run: (_i: JavaObject) => {
        throw new Error("newError");
      },
    });

    const runInterface = java.newInstanceSync("RunInterface");
    try {
      runInterface.runWithReturnSync(myProxy);
      throw new Error("Exception was not thrown");
    } catch (e) {
      expectJavaError(e);
      expect(e.cause.getClassSync().getNameSync()).toBe("java.lang.RuntimeException");
      expect(e.message).toMatch(/Caused by: node\.NodeJsException:.*newError/);
    }
  });

  test("invocationHandler", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {
      run: (i: number) => {
        return i + 2;
      },
    });

    const result = myProxy.invocationHandler.run(42);

    expect(result).toBe(44);
  });

  test("unref", () => {
    const myProxy = java.newProxy("RunInterface$InterfaceWithReturn", {
      run: (i: number) => {
        return i + 1;
      },
    });

    myProxy.unref();

    try {
      myProxy.invocationHandler.run(42);
    } catch (e) {
      expectJavaError(e);
      expect(e.message).toBe("dynamicProxyData has been destroyed or corrupted");
    }

    // call again
    myProxy.unref();
  });
});
