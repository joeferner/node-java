[![Build Status](https://travis-ci.org/joeferner/node-java.png)](https://travis-ci.org/joeferner/node-java)

# java

Bridge API to connect with existing Java APIs.

[Google Groups Discussion Forum](https://groups.google.com/forum/#!forum/node-java)

###Other projects that might be helpful

* [node-java-maven](https://github.com/joeferner/node-java-maven) - manages your node-java classpath by using maven dependency mangement.

## Installation

```bash
$ npm install java
```

Notes:

* node-gyp requires python 2.x not python 3.x. See https://github.com/TooTallNate/node-gyp/issues/155 for more details.
* If you see an error such as "Call to 'node findJavaHome.js' returned exit status 1"
      Try running `node findJavaHome.js` in the node-java directory to see the full failure message.
* If you are having problems finding 'jni.h'. Make sure you have the JDK installed not just the JRE. If you are using
      OpenJDK you want the openjdk-7-jdk package, not openjdk-7-jre.  _Mavericks users see [Issue #86](https://github.com/nearinfinity/node-java/issues/86) if you run into this._

### Installation OSX

* If you run into strange runtime issues, it could be because the Oracle JDK does not advertise itself as available for JNI.  See [Issue 90](https://github.com/joeferner/node-java/issues/90#issuecomment-45613235) for more details and manual workarounds.  If this does occur for you, please update the issue.

### Installation Windows

For 64 bit installs with 32 bit node:
* you need the 32 bit JDK, with the 64 bit JDK you will see LNK2001 errormessages (http://stackoverflow.com/questions/10309304/what-library-to-link-to-on-windows-7-for-jni-createjavavm).
* when using the windows SDK 7.1 command prompt (64 bits) be sure to setenv.cmd /Release /x86

If you get `ENOENT` errors looking for `<nodepath>\node_modules\node-gyp\..`, ensure you have node-gyp installed as a global nodule:

```bash
npm install -g node-gyp
```

If you get `D9025` warnings and `C1083` errors when looking for `.sln` or `.h` files, be sure you've got the `node-gyp`'s dependencies, [as explained here](https://github.com/joeferner/node-java#installation).

### Installation ARM (Raspberry Pi)

```bash
GYP_DEFINES="armv7=0" CCFLAGS='-march=armv6' CXXFLAGS='-march=armv6' npm install java
```

## Manual compile (Using node-gyp)

```bash
./compile-java-code.sh
node-gyp configure build
npm test
```

_NOTE: You will need node-gyp installed using "npm install -g node-gyp"_

### Java 1.8 support

Manual compilation for Java 1.8 support requires additional steps:

```bash
./compile-java-code.sh
./compile-java8-code.sh
node-gyp configure build
npm test
npm test8
```

Java 1.8 language features can be used in Java classes only if a Java 1.8 JRE is available. The script compile-java8-code.sh is used only to compile java classes used in the 'test8' unit tests, but these classes are checked into the test8/ directory. Note that unit tests in the test8/ directory will pass (by design) if run against a Java 1.7 JRE, provided that a java.lang.UnsupportedClassVersionError is caught with the message 'Unsupported major.minor version 52.0' (the expected behavior when Java 1.8 language features are used in an older JRE).

## Installation node-webkit

```bash
npm install -g nw-gyp
npm install java
cd node_modules/java
nw-gyp configure --target=0.10.5
nw-gyp build
```

_See testIntegration/webkit for a working example_

## Using node-java in existing maven projects

When using node-java in existing maven projects, all the dependencies and the class files of the project have to be pushed to the classpath.

One possible solution would be:

Issue the command:

> mvn dependency:copy-dependencies

Then create the following module javaInit:

```javascript
"use strict";
var fs = require("fs");
var java = require("java");
var baseDir = "./target/dependency";
var dependencies = fs.readdirSync(baseDir);

dependencies.forEach(function(dependency){
    java.classpath.push(baseDir + "/" + dependency);
})

java.classpath.push("./target/classes");
java.classpath.push("./target/test-classes");

exports.getJavaInstance = function() {
    return java;
}
```

and then in the consuming class write:

```javascript

var javaInit = require('./javaInit');
var java = javaInit.getJavaInstance();

//your code goes here
```



## Quick Examples

```javascript
var java = require("java");
java.classpath.push("commons-lang3-3.1.jar");
java.classpath.push("commons-io.jar");

var list = java.newInstanceSync("java.util.ArrayList");

java.newInstance("java.util.ArrayList", function(err, list) {
  list.addSync("item1");
  list.addSync("item2");
});

var ArrayList = java.import('java.util.ArrayList');
var list = new ArrayList();
list.addSync('item1');
```

### Create a char array

```javascript
var charArray = java.newArray("char", "hello world\n".split(''));
```

### Create a byte array

```javascript
var byteArray = java.newArray(
  "byte",
  "hello world\n"
    .split('')
    .map(function(c) { return java.newByte(str.charCodeAt(c)); });
```

### Using java.lang.Long and long

JavaScript only supports 32-bit integers. Because of this java longs must be treated specially.
When getting a long result the value may be truncated. If you need the original value there is
a property off of the result called "longValue" which contains the un-truncated value as a string.
If you are calling a method that takes a long you must create it using [java.newInstance](#javaNewInstance).

```javascript
var javaLong = java.newInstanceSync("java.lang.Long", 5);
console.log('Possibly truncated long value: ' + javaLong);
console.log('Original long value (as a string): ' + javaLong.longValue);
java.callStaticMethodSync("Test", "staticMethodThatTakesALong", javaLong);
```

### Exceptions

Exceptions from calling methods either caught using JavaScript try/catch block or passed
to a callback as the first parameter may have a property named "cause" which has a reference
to the Java Exception object which caused the error.

```javascript
try {
  java.methodThatThrowsExceptionSync();
} catch(ex) {
  console.log(ex.cause.getMessageSync());
}
```

### AsyncOptions: control over the generation of sync, async & promise method variants.

As of release 0.4.5 it became possible to create async methods that return promises by setting the asyncOptions property of the java object. With release 0.4.7 this feature is extended to allow changing the suffix assigned for sync and async method variants, and to further configure this module to optionally omit generation of any of these variants.

Example:

```javascript
var java = require("java");
java.asyncOptions = {
  asyncSuffix: undefined,     // Don't generate node-style methods taking callbacks
  syncSuffix: "",              // Sync methods use the base name(!!)
  promiseSuffix: "Promise",   // Generate methods returning promises, using the suffix Promise.
  promisify: require("when/node").lift
};
java.classpath.push("commons-lang3-3.1.jar");
java.classpath.push("commons-io.jar");

java.import("java.util.ArrayList"); // see NOTE below

java.newInstancePromise("java.util.ArrayList")
    .then(function(list) { return list.addPromise("item1"); })
    .then(function(list) { return list.addPromise("item2"); })
    .catch(function(err) { /* handle error */ });
```

#### NOTES:
* If you want the defacto standard behavior, simply don't set java.asyncOptions.
* If you do provide asyncOptions, be aware that this module will not generate method variants of a given flavor if you don't provide a string value for the corresponding suffix (`asyncSuffix`, `syncSuffix`, `promiseSuffix`). In the example above, the application is configured to omit the method variants using node-style async callback functions.
* If you provide `asyncOptions.promiseSuffix` then you must also set `asyncOptions.promisify` to a function that *promisifies* a node-style async function. I.e. the provided function must take as input a function whose last argument is a node callback function, and it must return an equivalent promise-returning function. Several Promises/A+ libraries provide such functions, but it may be necessary to provide a wrapper function. See `testHelpers.js` for an example.
* If you provide `asyncOptions.promisify` then you must provide a *non-empty* string for `asyncOptions.promiseSuffix`.
* Either (but not both) `asyncSuffix` or `syncSuffix` can be the empty string. If you want the defacto standard behavior for no suffix on async methods, you must provide an empty string for `asyncSuffix`.
* We've tested promises with five Promises/A+ implementations. See `testHelpers.js` for more information.
* NOTE: Due to specifics of initialization order, the methods  `java.newInstancePromise`, `java.callMethodPromise`, and `java.callStaticMethodPromise` are not available until the JVM has been created. You may need to call some other java method such as `java.import()` to finalize java initialization, or even better, the function `java.ensureJVM()`.

##### Special note about the exported module functions `newInstance`, `callMethod`, and `callStaticMethod`.
These methods come in both async and sync variants. If you provide the `promisify` and `promiseSuffix` attributes in asyncOptions then you'll also get the Promises/A+ variant for these three functions. However, if you change the defacto
conventions for the `syncSuffix` (i.e. 'Sync') and/or `asyncSuffix` (i.e. '') it will not affect the naming for these three functions. I.e. no matter what you specify in asyncOptions, the async variants are named `newInstance`, `callMethod`, and `callStaticMethod`, and the sync variants are named `newInstanceSync`, `callMethodSync`, and `callStaticMethodSync`.

## Varargs support

With v0.5.0 node-java now supports methods with variadic arguments (varargs). Prior to v0.5.0,
a javascript call to a Java varargs method had to construct an array of the variadic arguments using `java.newArray()`. With v0.5.0 javascript applications can simply use the variadic style.

In most cases it is still acceptable to use `java.newArray()`. But it is now possible to pass a plain javascript array, or use the variadic style. For example, consider these snippets from the unit test file `test/varargs-test.js`:

```
    test.equal(Test.staticVarargsSync(5, 'a', 'b', 'c'), '5abc');
    test.equal(Test.staticVarargsSync(5, ['a', 'b', 'c']), '5abc');
    test.equal(Test.staticVarargsSync(5, java.newArray('java.lang.String', ['a', 'b', 'c'])), '5abc');

```

Note that when passing a Javascript array (e.g. `['a', 'b', 'c']`) for a varargs parameter, node-java must infer the Java type of the array. If all of the elements are of the same javascript primitive type (`string` in this example) then node-java will create a Java array of the corresponding type (e.g. `java.lang.String`). The Java types that node-java can infer are: `java.lang.String`, `java.lang.Boolean`, `java.lang.Integer`, `java.lang.Long`, and `java.lang.Double`. If an array has a mix of `Integer`, `Long`, and `Double`, then the inferred type will be `java.lang.Number`. Any other mix will result in an inferred type of `java.lang.Object`.

## JVM Creation

With v0.5.1 a new API is available to make it easier for a complex application to have full control over JVM creation. In particular, it is now easier to compose an application from several modules, each of which must add to the Java classpath and possibly do other operations just before or just after the JVM has been created. See the methods [ensureJvm](#javaEnsureJvm) and [registerClient](#javaRegisterClient). See also several of the tests in the testAsyncOptions directory.

# Release Notes

### v0.5.0

* Support for varargs. This change is not 100% backwards compatible, but the fix is generally easy and results in more natural code.

### v0.2.0

* java.lang.Long and long primitives are handled better. See
  \([Issue #37](https://github.com/nearinfinity/node-java/issues/37)\) and
  \([Issue #40](https://github.com/nearinfinity/node-java/issues/40)\).

# Index

## java
 * [classpath](#javaClasspath)
 * [options](#javaOptions)
 * [import](#javaImport)
 * [newInstance](#javaNewInstance)
 * [instanceOf](#javaInstanceOf)
 * [callStaticMethod](#javaCallStaticMethod)
 * [callMethod](#javaCallMethod)
 * [getStaticFieldValue](#javaGetStaticFieldValue)
 * [setStaticFieldValue](#javaSetStaticFieldValue)
 * [newArray](#javaNewArray)
 * [newByte](#javaNewByte)
 * [newShort](#javaNewShort)
 * [newLong](#javaNewLong)
 * [newChar](#javaNewChar)
 * [newDouble](#javaNewDouble)
 * [newFloat](#javaNewFloat)
 * [newProxy](#javaNewProxy)
 * [isJvmCreated](#javaIsJvmCreated)
 * [registerClient](#javaRegisterClient)
 * [registerClientP](#javaRegisterClientP)
 * [ensureJvm](#javaEnsureJvm)

## java objects
 * [Call Method](#javaObjectCallMethod)
 * [Get/Set Field](#javaObjectGetSetField)

# API Documentation

<a name="java"/>
## java

<a name="javaClasspath" />
**java.classpath**

Array of paths or jars to pass to the creation of the JVM.

All items must be added to the classpath before calling any other node-java methods.

__Example__

    java.classpath.push('commons.io.jar');
    java.classpath.pathDir('lib');

<a name="javaOptions" />
**java.options**

Array of options to pass to the creation of the JVM.

All items must be added to the options before calling any other node-java methods.

__Example__

    java.options.push('-Djava.awt.headless=true');
    java.options.push('-Xmx1024m');

<a name="javaImport" />
**java.import(className)**

Loads the class given by className such that it acts and feels like a javascript object.

__Arguments__

 * className - The name of the class to create. For subclasses seperate using a '$' (eg. com.nearinfinty.MyClass$SubClass)

__Example__

    var Test = java.import('Test');
    Test.someStaticMethodSync(5);
    console.log(Test.someStaticField);

    var value1 = Test.NestedEnum.Value1;

    var test = new Test();
    list.instanceMethodSync('item1');

<a name="javaNewInstance" />
**java.newInstance(className, [args...], callback)**

**java.newInstanceSync(className, [args...]) : result**

Creates an instance of the specified class. If you are using the sync method an exception will be throw if an error occures,
otherwise it will be the first argument in the callback.

__Arguments__

 * className - The name of the class to create. For subclasses seperate using a '$' (eg. com.nearinfinty.MyClass$SubClass)
 * callback(err, item) - Callback to be called when the class is created.

__Example__

    var list = java.newInstanceSync("java.util.ArrayList");

    java.newInstance("java.util.ArrayList", function(err, list) {
      if(err) { console.error(err); return; }
      // new list
    });

<a name="javaInstanceOf" />
**java.instanceOf(javaObject, className)**

Determines of a javaObject is an instance of a class.

__Arguments__

 * javaObject - Instance of a java object returned from a method or from newInstance.
 * className - A string class name.

__Example__

    var obj = java.newInstanceSync("my.package.SubClass");

    if(java.instanceOf(obj, "my.package.SuperClass")) {
      console.log("obj is an instance of SuperClass");
    }

<a name="javaCallStaticMethod" />
**java.callStaticMethod(className, methodName, [args...], callback)**

**java.callStaticMethodSync(className, methodName, [args...]) : result**

Calls a static method on the specified class. If you are using the sync method an exception will be throw if an error occures,
otherwise it will be the first argument in the callback.

__Arguments__

 * className - The name of the class to call the method on. For subclasses seperate using a '$' (eg. com.nearinfinty.MyClass$SubClass)
 * methodName - The name of the method to call. The method name can include the full signature (see [Getting the full method signature](#getFullMethodSignature)).
 * callback(err, item) - Callback to be called when the class is created.

__Example__

    var result = java.callStaticMethodSync("com.nearinfinty.MyClass", "doSomething", 42, "test");

    java.callStaticMethod("com.nearinfinty.MyClass", "doSomething", 42, "test", function(err, results) {
      if(err) { console.error(err); return; }
      // results from doSomething
    });

<a name="javaCallMethod" />
**java.callMethod(instance, methodName, [args...], callback)**

**java.callMethodSync(instance, methodName, [args...]) : result**

Calls a method on the specified instance. If you are using the sync method an exception will be throw if an error occures,
otherwise it will be the first argument in the callback.

__Arguments__

 * instance - An instance of the class from newInstance.
 * methodName - The name of the method to call. The method name can include the full signature (see [Getting the full method signature](#getFullMethodSignature)).
 * callback(err, item) - Callback to be called when the class is created.

__Example__

    var instance = java.newInstanceSync("com.nearinfinty.MyClass");

    var result = java.callMethodSync("com.nearinfinty.MyClass", "doSomething", 42, "test");

    java.callMethodSync(instance, "doSomething", 42, "test", function(err, results) {
      if(err) { console.error(err); return; }
      // results from doSomething
    });

<a name="javaGetStaticFieldValue" />
**java.getStaticFieldValue(className, fieldName)**

Gets a static field value from the specified class.

__Arguments__

 * className - The name of the class to get the value from. For subclasses seperate using a '$' (eg. com.nearinfinty.MyClass$SubClass)
 * fieldName - The name of the field to get the value from.

__Example__

    var data = java.getStaticFieldValue("com.nearinfinty.MyClass", "data");

<a name="javaSetStaticFieldValue" />
**java.setStaticFieldValue(className, fieldName, newValue)**

Sets a static field value on the specified class.

__Arguments__

 * className - The name of the class to set the value on. For subclasses seperate using a '$' (eg. com.nearinfinty.MyClass$SubClass)
 * fieldName - The name of the field to set the value on.
 * newValue - The new value to assign to the field.

__Example__

    java.setStaticFieldValue("com.nearinfinty.MyClass", "data", "Hello World");

<a name="javaNewArray" />
**java.newArray(className, values[])**

Creates a new java array of type class.

__Arguments__

 * className - The name of the type of array elements. For subclasses seperate using a '$' (eg. com.nearinfinty.MyClass$SubClass)
 * values - A javascript array of values to assign to the java array.

__Example__

    var newArray = java.newArray("java.lang.String", ["item1", "item2", "item3"]);

<a name="javaNewByte" />
**java.newByte(val)**

Creates a new java byte. This is needed because javascript does not have the concept of a byte.

__Arguments__

 * val - The value of the java byte.

__Example__

    var b = java.newByte(12);

<a name="javaNewShort" />
**java.newShort(val)**

Creates a new java short. This is needed because javascript does not have the concept of a short.

__Arguments__

 * val - The value of the java short.

__Example__

    var s = java.newShort(12);

<a name="javaNewLong" />
**java.newLong(val)**

Creates a new java long. This is needed because javascript does not have the concept of a long.

__Arguments__

 * val - The value of the java long.

__Example__

    var s = java.newLong(12);

<a name="javaNewChar" />
**java.newChar(val)**

Creates a new java char. This is needed because javascript does not have the concept of a char.

__Arguments__

 * val - The value of the java char.

__Example__

    var ch = java.newChar('a');

<a name="javaNewDouble" />
**java.newDouble(val)**

Creates a new java double. This is needed to force javascript's number to a double to call some methods.

__Arguments__

 * val - The value of the java double.

__Example__

    var d = java.newDouble(3.14);

<a name="javaNewFloat" />
**java.newFloat(val)**

Creates a new java float. This is needed to force javascript's number to a float to call some methods.

__Arguments__

 * val - The value of the java float.

__Example__

    var f = java.newFloat(3.14);

<a name="javaNewProxy" />
**java.newProxy(interfaceName, functions)**

Creates a new java Proxy for the given interface. Functions passed in will run on the v8 main thread and not a new thread.

The returned object has a method unref() which you can use to free the object for
garbage collection.

__Arguments__

 * interfaceName - The name of the interface to proxy. For subclasses seperate using a '$' (eg. com.nearinfinty.MyClass$SubClass)
 * functions - A hash of functions matching the function in the interface.

__Example__

    var myProxy = java.newProxy('java.lang.Runnable', {
      run: function () {
        // This is actually run on the v8 thread and not the new java thread
        console.log("hello from thread");
      }
    });

    var thread = java.newInstanceSync("java.lang.Thread", myProxy);
    thread.start();

<a name="javaisJvmCreated" />
**java.isJvmCreated()**

Returns true if the JVM has been created. The JVM can only be created once.

<a name="javaRegisterClient" />
**java.registerClient(before, after)**

Register that a client wants to be called back immediately before and/or immediately after the JVM is created. If used, this function must be called before the JVM has been created. The before function is typically used to add to the classpath. The function may execute asynchronous operations (such as a async glob function). The after function is sometimes useful for doing one-time initialization that requires the JVM to first be initialized. If either function is unnecessary, use `null` or `undefined`. See also `registerClientP` and `ensureJvm`. See the unit tests in `testAsyncOptions` for examples.

<a name="javaRegisterClientP" />
**java.registerClientP(before, after)**

Like java.registerClient, but before and after are assumed to be functions returning promises.

<a name="javaEnsureJvm" />
**java.ensureJvm(callback)**

If the JVM has not yet been created, execute the full JVM initialization process, then call callback function when initialization is complete. If the JVM has been created, just call the callback. Note that the full initialization process includes: 1) executing all registered client *before* hooks, 2) creating the JVM, then 3) executing all registered client *after* hooks.



<a name="javaObject"/>
## java object

<a name="javaObjectCallMethod" />
**obj._methodName_([args...], callback)**

**obj._methodNameSync_([args...]) : result**

Once you have a java object either by creating a new instance or as a result of a method call you can then call methods on that object.
All public, non-static methods are exposed in synchronous and asynchronous flavors.

__Arguments__

 * args - The arguments to pass to the method.
 * callback(err, item) - Callback to be called when the method has completed.

__Example__

    var list = java.newInstanceSync("java.util.ArrayList");
    list.addSync("item1");
    list.add("item2", function(err, result) {
      if(err) { console.error(err); return; }
    });

<a name="javaObjectGetSetField" />
**obj._fieldName_ = val**

**val = obj._fieldName_**

Once you have a java object either by creating a new instance or as a result of a method call you can get instance
field values.

__Example__

    var list = java.newInstanceSync("com.nearinfinty.MyClass");
    list.data = "test";
    var data = list.data;

<a name="getFullMethodSignature" />
# Getting the Full Method Signature

Run `javap -s -classpath <your-class-path> <your-class-name>`. Find the method name you are looking for. For example:

```
public int methodAmbiguous(java.lang.Double);
  Signature: (Ljava/lang/Double;)I
```

The full method signature would be `methodAmbiguous(Ljava/lang/Double;)I`.

If you have grep, a shortcut is `javap -s -classpath . my.company.MyClass | grep -A1 myMethodName`.

# Signal Handling

The JVM intercepts signals (Ctrl+C, etc.) before node/v8 gets to handle them. To fix this there are a couple options.

## Signal Handling Option 1

One option to capture these events is to add the following flag:

```javascript
java.options.push('-Xrs');
```

As `man java` says, the `-Xrs` flag will “reduce usage of operating-system signals by [the] Java virtual machine (JVM)”, to avoid issues when developing “applications that embed the JVM”.

## Signal Handling Option 2

Hook into the runtime shutdown hook.

First create a java wrapper around the Runtime.addShutdownHook method to allow using a proxy object.

```java
public class ShutdownHookHelper {
  public static void setShutdownHook(final Runnable r) {
    Runtime.getRuntime().addShutdownHook(new Thread() {
      @Override
      public void run() {
        r.run();
      }
    });
  }
}
```

Compile ShutdownHookHelper and then use it as follows.

```javascript
var java = require('./');
java.classpath.push('.');
var ShutdownHookHelper = java.import('ShutdownHookHelper');

ShutdownHookHelper.setShutdownHookSync(java.newProxy('java.lang.Runnable', {
  run: function () {
    console.log("do shutdown stuff here instead.");
  }
}));
```

# Object lifetime

When you call a Java method through node-java, any arguments (V8/JavaScript objects) will be converted to Java objects  on the v8 main thread via a call to v8ToJava (found in utils.cpp). The JavaScript object is not held on to and can be garbage collected by v8. If this is an async call, the reference count on the Java objects will be incremented. The Java method will be invoked in a node.js async thread (see uv_queue_work). When the method returns, the resulting object will be returned to the main v8 thread and converted to JavaScript objects via a call to javaToV8 and the Java object's reference count will then be decremented to allow for garbage collection. The resulting v8 object will then be returned to the callers callback function.

# Static member name conficts ('name', 'arguments', 'caller')

The Javscript object returned by `java.import(classname)` is a Javascript constructor Function, implemented such that you can create instances of the Java class. For example:

```javascript
var Test = java.import('Test');
var test = new Test();

Test.someStaticMethod(function(err, result) { ... });

var value1 = Test.NestedEnum.Value1;
```

But Javascript reserves a few property names of Function objects: `name`, `arguments`, and `caller`. If your class has public static members (either methods or fields) with these names, node-java is unable to create the necessary property to implement the class's API. For example, suppose your class `Test` implements a static method named `caller`, or has a `NestedEnum` with a value `name`:

```java
public class Test {
    ...
    public static String caller() { return "something"; }
    public enum NestedEnum { foo, name };
}
```

In Javascript, you would expect to be able to use those static members like this:

```javascript
var Test = java.import('Test');
Test.caller(function(err, result) { ... });  // ERROR
var value = Test.NestedEnum.name;  // ERROR
```

Node-java can't create those properties, so the above code won't work. Instead, node-java appends a suffix to the name. The default suffix is simpy an underscore `_`, but you can change the suffix using asyncOptions:

```javascript
var java = require('java');

java.asyncOptions = {
  asyncSuffix: "",
  syncSuffix: "Sync",
  ifReadOnlySuffix: "_alt"
};

var Test = java.import('Test');
Test.caller_alt(function(err, result) { ... });  // OK
var value = Test.NestedEnum.name_alt;  // OK
```

# Troubleshooting

## Error: Cannot find module '../build/jvm_dll_path.json'

Either postInstall.js didn't run or there was a problem detecting java. Try running postInstall.js manually.

## License

(The MIT License)

Copyright (c) 2012 Near Infinity Corporation

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
