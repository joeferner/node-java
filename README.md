# java

Bridge API to connect with existing Java APIs.

## Installation Linux/Windows

```bash
$ export JAVA_HOME=/usr/local/share/jdk1.6.0_30
$ npm install java
```

Notes:
* node-gyp requires python 2.x not python 3.x. See https://github.com/TooTallNate/node-gyp/issues/155 for more details.
* If you see an error such as "No rule to make target '/opt/jdk1.7.0_09/jre/lib/amd64/server/libjvm.so', needed by 'build/depsVerified'.  Stop."
      this means that your JAVA_HOME is not set correctly and you need to verify the location of libjvm.so or jvm.lib.
* If you see an error such as "Error: The specified module could not be found. 
      xxxxxx\node_modules\java\build\Release\nodejavabridge_bindings.node".
      Ensure the directory where jvm.dll exists is in your PATH. (e.g. C:\Program Files (x86)\Java\jdk1.6.0_18\jre\bin\client).
      This path cannot have quotes.


## Installation Mac

```bash
$ npm install java
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

# Release Notes

### v0.2.0

* java.lang.Long and long primitives are handled better. See
  \([Issue #37](https://github.com/nearinfinity/node-java/issues/37)\) and
  \([Issue #40](https://github.com/nearinfinity/node-java/issues/40)\).

# Index

## java
 * [import](#javaImport)
 * [newInstance](#javaNewInstance)
 * [callStaticMethod](#javaCallStaticMethod)
 * [getStaticFieldValue](#javaGetStaticFieldValue)
 * [setStaticFieldValue](#javaSetStaticFieldValue)
 * [newArray](#javaNewArray)
 * [newByte](#javaNewByte)
 * [newProxy](#javaNewProxy)

## java objects
 * [Call Method](#javaObjectCallMethod)
 * [Get/Set Field](#javaObjectGetSetField)

# API Documentation

<a name="java"/>
## java

<a name="javaImport" />
**java.import(className)**

Loads the class given by className such that it acts and feels like a javascript object.

__Arguments__

 * className - The name of the class to create. For subclasses seperate using a '$' (eg. com.nearinfinty.MyClass$SubClass)

__Example__

    var Test = java.import('Test');
    Test.someStaticMethodSync(5);
    console.log(Test.someStaticField);

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

<a name="javaCallStaticMethod" />
**java.callStaticMethod(className, methodName, [args...], callback)**

**java.callStaticMethodSync(className, methodName, [args...]) : result**

Calls a static method on the specified class. If you are using the sync method an exception will be throw if an error occures,
otherwise it will be the first argument in the callback.

__Arguments__

 * className - The name of the class to call the method on. For subclasses seperate using a '$' (eg. com.nearinfinty.MyClass$SubClass)
 * methodName - The name of the method to call.
 * callback(err, item) - Callback to be called when the class is created.

__Example__

    var result = java.callStaticMethodSync("com.nearinfinty.MyClass", "doSomething", 42, "test");

    java.callStaticMethod("com.nearinfinty.MyClass", "doSomething", 42, "test", function(err, results) {
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

    java.getStaticFieldValue("com.nearinfinty.MyClass", "data", "Hello World");

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

<a name="javaNewProxy" />
**java.newProxy(interfaceName, functions)**

Creates a new java Proxy for the given interface. Functions passed in will run on the v8 main thread and not a new thread.

The returned object has two methods ref() and unref() which you can use to maintain references to prevent premature
garbage collection. You must call these methods to ensure the proxy stays around.

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
