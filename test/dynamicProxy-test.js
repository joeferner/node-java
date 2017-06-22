'use strict';

var java = require("../testHelpers").java;
var nodeunit = require("nodeunit");
var util = require("util");

exports['Dynamic Proxy'] = nodeunit.testCase({
  "0 Arguments": function (test) {
    var callCount = 0;

    var myProxy = java.newProxy('RunInterface$Interface0Arg', {
      run: function () {
        callCount++;
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    runInterface.run0ArgsSync(myProxy);

    test.equals(callCount, 2);

    test.done();
  },

  "1 Arguments": function (test) {
    var runData = '';

    var myProxy = java.newProxy('RunInterface$Interface1Arg', {
      run: function (str) {
        runData += str;
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    runInterface.run1ArgsSync(myProxy);

    test.equals(runData, 'test1test1');

    test.done();
  },

  "1 Arguments with return data": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
      run: function (i) {
        return i + 1;
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    var result = runInterface.runWithReturnSync(myProxy);

    test.equals(result, 43);

    test.done();
  },

  "Listener test": function (test) {
    var runData = '';

    var myProxy = java.newProxy('ListenerInterface', {
      onEvent: function (list, runtime) {
        runData = 'onEvent';
      }
    });

    var listenerTester = java.newInstanceSync("ListenerTester");
    listenerTester.setListenerSync(myProxy);
    listenerTester.raiseEventSync();

    test.equals(runData, 'onEvent');

    test.done();
  },  

  "thread": function (test) {
    var callCount = 0;

    var myProxy = java.newProxy('java.lang.Runnable', {
      run: function () {
        callCount++;
      }
    });

    var thread = java.newInstanceSync("java.lang.Thread", myProxy);
    thread.startSync();

    var timeout = 50;

    function waitForThread() {
      if (callCount === 1) {
        return test.done();
      }
      timeout--;
      if (timeout < 0) {
        test.done(new Error("Timeout"));
      }
      setTimeout(waitForThread, 100);
    }

    waitForThread();
  },

  "thread issue #143": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
      run: function (i) {
        return i - 1;
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    runInterface.runInAnotherThread(myProxy, function(err, result) {
        test.equals(result, 45);

        test.done();
    });
  },

  "java equals()": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
    });

    var runInterface = java.newInstanceSync("RunInterface");
    var result = runInterface.runEqualsSync(myProxy);

    test.equals(result, false);

    test.done();
  },

  "java equals() same instance": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
    });

    var runInterface = java.newInstanceSync("RunInterface");
    runInterface.setInstanceSync(myProxy);
    var result = runInterface.runEqualsInstanceSync(myProxy);

    test.equals(result, true);

    test.done();
  },

  "java equals() different instance": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {});
    var myProxy2 = java.newProxy('RunInterface$InterfaceWithReturn', {});

    var runInterface = java.newInstanceSync("RunInterface");
    runInterface.setInstanceSync(myProxy);
    var result = runInterface.runEqualsInstanceSync(myProxy2);

    test.equals(result, false);

    test.done();
  },

  "js equals()": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
      equals: function (obj) {
        return true;
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    var result = runInterface.runEqualsSync(myProxy);

    test.equals(result, true);

    test.done();
  },

  "java hashCode()": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
    });

    var runInterface = java.newInstanceSync("RunInterface");
    var result = runInterface.runHashCodeSync(myProxy);
    var result2 = runInterface.runHashCodeSync(myProxy);
    var systemHashCode = java.callStaticMethodSync("java.lang.System", "identityHashCode", myProxy);

    test.equals(result, result2);
    test.equals(result, systemHashCode);

    test.done();
  },

  "js hashCode()": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
      hashCode: function() {
        return 1234;
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    var result = runInterface.runHashCodeSync(myProxy);

    test.equals(result, 1234);

    test.done();
  },

  "java toString()": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {});

    var runInterface = java.newInstanceSync("RunInterface");
    var result = runInterface.runToStringSync(myProxy);

    test.equals(result, "[object Object]");

    test.done();
  },

  "js toString()": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
      toString: function() {
        return "myRunInterface";
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    var result = runInterface.runToStringSync(myProxy);

    test.equals(result, "myRunInterface");

    test.done();
  },

  "js string error": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
      run: function (i) {
        throw 'myError';
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    try {
      runInterface.runWithReturnSync(myProxy);
      test.fail("Exception was not thrown");
    } catch (e) {
      test.equals(e.cause.getClassSync().getNameSync(), "java.lang.RuntimeException");
      test.ok(/Caused by: node\.NodeJsException:.*myError/.test(e.message));
    }

    test.done();
  },

  "js Error": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
      run: function (i) {
        throw new Error('newError');
      }
    });

    var runInterface = java.newInstanceSync("RunInterface");
    try {
        runInterface.runWithReturnSync(myProxy);
        test.fail("Exception was not thrown");
    } catch (e) {
        test.equals(e.cause.getClassSync().getNameSync(), "java.lang.RuntimeException");
        test.ok(/Caused by: node\.NodeJsException:.*newError/.test(e.message));
    }

    test.done();
  },

  "invocationHandler": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
      run: function (i) {
        return i + 2;
      }
    });

    var result = myProxy.invocationHandler.run(42);

    test.equals(result, 44);

    test.done();
  },

  "unref": function (test) {
    var myProxy = java.newProxy('RunInterface$InterfaceWithReturn', {
      run: function (i) {
        return i + 1;
      }
    });

    myProxy.unref();

    try {
      myProxy.invocationHandler.run(42);
    } catch (e) {
      test.equals(e.message, "dynamicProxyData has been destroyed or corrupted");
    }

    // call again
    myProxy.unref();
    test.done();
  }
});
