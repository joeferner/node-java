# java

Bridge API to connect with existing Java APIs.

## Installation Linux

```bash
$ export JAVA_HOME=/usr/local/share/jdk1.6.0_30
$ npm install java
```

## Installation Windows

* [Install node.js from source](https://github.com/joyent/node/wiki/Installation) (using: vcbuild.bat release)
* The directory where jvm.dll exists must be in the PATH. (e.g. C:\Program Files (x86)\Java\jdk1.6.0_18\jre\bin\client)
* Open a Visual Studio command prompt.
* Your Java must be the same architecture as node. By default on windows this is 32-bit.

```bash
$ set PATH=%PATH%;C:\Program Files (x86)\Java\jdk1.6.0_18\jre\bin\client
$ set JAVA_HOME=C:\Program Files (x86)\Java\jdk1.6.0_18\
$ set NODE_HOME=C:\dev\node-v0.6.8-0
$ npm install java
```

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

# Index

## java
 * [import](#javaImport)
 * [newInstance](#javaNewInstance)
 * [callStaticMethod](#javaCallStaticMethod)
 * [getStaticFieldValue](#javaGetStaticFieldValue)
 * [setStaticFieldValue](#javaSetStaticFieldValue)
 * [newArray](#javaNewArray)
 * [newByte](#javaNewByte)

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
