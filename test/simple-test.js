var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");

exports['Simple'] = nodeunit.testCase({
  "test classpath commons lang": function(test) {
    var result = java.callStaticMethodSync("org.apache.commons.lang3.ObjectUtils", "toString", "test");
    console.log("org.apache.commons.lang3.ObjectUtils.toString:", result);
    test.equal(result, "test");
    test.done();
  },

  "test static calls": function(test) {
    var result = java.callStaticMethodSync("java.lang.System", "currentTimeMillis");
    console.log("currentTimeMillis:", result);
    test.ok(result);
    test.done();
  },

  "test static calls single argument": function(test) {
    var result = java.callStaticMethodSync("java.lang.System", "getProperty", "os.version");
    console.log("os.version:", result);
    test.ok(result);
    test.done();
  },

  "test method does not exists (sync)": function(test) {
    test.throws(
      function() {
        java.callStaticMethodSync("java.lang.System", "badMethod");
      }
    );
    test.done();
  },

  "test method does not exists (async)": function(test) {
    java.callStaticMethod("java.lang.System", "badMethod", function(err, result) {
      if(err) { test.done(); return; }
      test.done(new Error("should throw exception"));
    });
  },

  "create an instance of a class and call methods (getName) (async)": function(test) {
    java.newInstance("java.util.ArrayList", function(err, list) {
      if(err) { console.log(err); return; }
      test.ok(list);
      if(list) {
        list.getClass(function(err, result) {
          if(err) { console.log(err); return; }
          result.getName(function(err, result) {
            if(err) { console.log(err); return; }
            test.equal(result, "java.util.ArrayList");
            test.done();
          });
        });
      }
    });
  },

  "create an instance of a class and call methods (getName) (sync)": function(test) {
    var list = java.newInstanceSync("java.util.ArrayList");
    test.equal(list.sizeSync(), 0);
    list.addSync("hello");
    list.addSync("world");
    test.equal(list.sizeSync(), 2);
    var item0 = list.getSync(0);
    test.equal(item0, "hello");
    var clazz = list.getClassSync();
    var result = clazz.getNameSync();
    test.equal(result, "java.util.ArrayList");
    test.done();
  },

  "create an instance of a class and call methods (size) (async)": function(test) {
    java.newInstance("java.util.ArrayList", function(err, list) {
      if(err) { console.log(err); return; }
      test.ok(list);
      if(list) {
        list.size(function(err, result) {
          if(err) { console.log(err); return; }
          test.equal(result, 0);
          test.done();
        });
      }
    });
  },

  "passing objects to methods": function(test) {
    var dataArray = "hello world\n".split('').map(function(c) { return java.newByte(c.charCodeAt(0)); });
    var data = java.newArray("byte", dataArray);
    //console.log("data", data.toStringSync());
    var stream = java.newInstanceSync("java.io.ByteArrayInputStream", data);
    //console.log("stream", stream);
    var reader = java.newInstanceSync("java.io.InputStreamReader", stream);
    //console.log("reader", reader);
    var bufferedReader = java.newInstanceSync("java.io.BufferedReader", reader);
    var str = bufferedReader.readLineSync();
    console.log("bufferedReader.readLineSync", str);
    test.equal(str, "hello world");
    test.done();
  },

  "method returning an array of ints sync": function(test) {
    var arr = java.callStaticMethodSync("Test", "getArrayOfInts");
    console.log(arr);
    test.done();
  },

  "method returning an array of bytes sync": function(test) {
    var arr = java.callStaticMethodSync("Test", "getArrayOfBytes");
    console.log(arr);
    test.done();
  },

  "method returning an array of bools sync": function(test) {
    var arr = java.callStaticMethodSync("Test", "getArrayOfBools");
    console.log(arr);
    test.done();
  },

  "method returning an array of doubles sync": function(test) {
    var arr = java.callStaticMethodSync("Test", "getArrayOfDoubles");
    console.log(arr);
    test.done();
  },

  "method returning an array of floats sync": function(test) {
    var arr = java.callStaticMethodSync("Test", "getArrayOfFloats");
    console.log(arr);
    test.done();
  },

  "method returning an array of longs sync": function(test) {
    var arr = java.callStaticMethodSync("Test", "getArrayOfLongs");
    arr = arr.map(function(l) {
      return l.toStringSync();
    });
    console.log(arr);
    test.done();
  }

});

