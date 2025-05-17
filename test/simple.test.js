import { describe, expect, test } from "vitest";
import { getJava } from "../testHelpers";

const java = getJava();

describe('Simple', () => {
  test("test classpath commons lang", () => {
    const result = java.callStaticMethodSync("org.apache.commons.lang3.ObjectUtils", "toString", "test");
    console.log("org.apache.commons.lang3.ObjectUtils.toString:", result);
    expect(result).toBe("test");
  });

  test("test adding to classpath after other calls are made", () => {
    java.callStaticMethodSync("java.lang.System", "currentTimeMillis");
    try {
      console.log('classpath', java.classpath);
      java.classpath = ["test/"];
      throw new Error("Exception should be thrown");
    } catch (e) {
      // ok
    }
  });

  test("test changing options after other calls are made", () => {
    java.callStaticMethodSync("java.lang.System", "currentTimeMillis");
    try {
      console.log('options', java.options);
      java.options = ["newoption"];
      throw new Error("Exception should be thrown");
    } catch (e) {
      // ok
    }
  });

  test("test changing nativeBindingLocation after other calls are made", () => {
    java.callStaticMethodSync("java.lang.System", "currentTimeMillis");
    try {
      console.log('nativeBindingLocation', java.nativeBindingLocation);
      java.nativeBindingLocation = "newNativeBindingLocation";
      throw new Error("Exception should be thrown");
    } catch (e) {
      // ok
    }
  });

  test("test static calls", () => {
    const result = java.callStaticMethodSync("java.lang.System", "currentTimeMillis");
    console.log("currentTimeMillis:", result);
    expect(result).toBeTruthy();
  });

  test("test static calls single argument", () => {
    const result = java.callStaticMethodSync("java.lang.System", "getProperty", "os.version");
    console.log("os.version:", result);
    expect(result).toBeTruthy();
  });

  test("test method does not exists (sync)", () => {
    expect(() => {
      java.callStaticMethodSync("java.lang.System", "badMethod");
    }).toThrow();
  });

  test("test method does not exists (async)", () => {
    java.callStaticMethod("java.lang.System", "badMethod", function (err, result) {
      if (err) {

        return;
      }
      test.done(new Error("should throw exception"));
    });
  });

  test("create an instance of a class and call methods (getName) (async)", async () => {
    await new Promise(resolve => {
      java.newInstance("java.util.ArrayList", (err, list) => {
        expect(err).toBeFalsy();
        expect(list).toBeTruthy();
        list.getClass((err, result) => {
          expect(err).toBeFalsy();
          result.getName((err, result) => {
            expect(err).toBeFalsy();
            expect(result).toBe("java.util.ArrayList");
            resolve();
          });
        });
      });
    });
  });

  test("create an instance of a class and call methods (getName) (sync)", () => {
    const list = java.newInstanceSync("java.util.ArrayList");
    expect(list.sizeSync()).toBe(0);
    list.addSync("hello");
    list.addSync("world");
    expect(list.sizeSync()).toBe(2);
    const item0 = list.getSync(0);
    expect(item0).toBe("hello");
    const clazz = list.getClassSync();
    const result = clazz.getNameSync();
    expect(result).toBe("java.util.ArrayList");
  });

  test("create an instance of a class and call methods (size) (async)", async () => {
    await new Promise((resolve) => {
      java.newInstance("java.util.ArrayList", (err, list) => {
        expect(err).toBeFalsy();
        expect(list).toBeTruthy();
        list.size((err, result) => {
          expect(err).toBeFalsy();
          expect(result).toBe(0);
          resolve();
        });
      });
    })
  });

  test("passing objects to methods", () => {
    const dataArray = "hello world\n".split('').map(function (c) { return java.newByte(c.charCodeAt(0)); });
    const data = java.newArray("byte", dataArray);
    const stream = java.newInstanceSync("java.io.ByteArrayInputStream", data);
    const reader = java.newInstanceSync("java.io.InputStreamReader", stream);
    const bufferedReader = java.newInstanceSync("java.io.BufferedReader", reader);
    const str = bufferedReader.readLineSync();
    expect(str).toBe("hello world");
  });

  test("method returning an array of ints sync", () => {
    const arr = java.callStaticMethodSync("Test", "getArrayOfInts");
    expect(arr.length).toBe(5);
    expect(arr[0]).toBe(1);
    expect(arr[1]).toBe(2);
    expect(arr[2]).toBe(3);
    expect(arr[3]).toBe(4);
    expect(arr[4]).toBe(5);
  });

  test("method returning an array of bytes sync", () => {
    const arr = java.callStaticMethodSync("Test", "getArrayOfBytes");
    expect(arr.length).toBe(5);
    expect(arr[0]).toBe(1);
    expect(arr[1]).toBe(2);
    expect(arr[2]).toBe(3);
    expect(arr[3]).toBe(4);
    expect(arr[4]).toBe(5);
  });

  test("method returning an array of bools sync", () => {
    const arr = java.callStaticMethodSync("Test", "getArrayOfBools");
    expect(arr.length).toBe(5);
    expect(arr[0]).toBe(true);
    expect(arr[1]).toBe(true);
    expect(arr[2]).toBe(false);
    expect(arr[3]).toBe(true);
    expect(arr[4]).toBe(false);
  });

  test("method returning an array of doubles sync", () => {
    const arr = java.callStaticMethodSync("Test", "getArrayOfDoubles");
    expect(arr.length).toBe(5);
    expect(arr[0]).toBe(1);
    expect(arr[1]).toBe(2);
    expect(arr[2]).toBe(3);
    expect(arr[3]).toBe(4);
    expect(arr[4]).toBe(5);
  });

  test("method returning an array of floats sync", () => {
    const arr = java.callStaticMethodSync("Test", "getArrayOfFloats");
    expect(arr.length).toBe(5);
    expect(arr[0]).toBe(1);
    expect(arr[1]).toBe(2);
    expect(arr[2]).toBe(3);
    expect(arr[3]).toBe(4);
    expect(arr[4]).toBe(5);
  });

  test("method returning an array of longs sync", () => {
    let arr = java.callStaticMethodSync("Test", "getArrayOfLongs");
    arr = arr.map(function (l) {
      return l.toStringSync();
    });
    expect(arr.length).toBe(5);
    expect(arr[0]).toBe('9223372036854775807');
    expect(arr[1]).toBe('-9223372036854775808');
    expect(arr[2]).toBe('3');
    expect(arr[3]).toBe('4');
    expect(arr[4]).toBe('5');
  });

  test("method returning a string (Unicode BMP)", () => {
    const s = java.callStaticMethodSync("Test", "getUnicodeBMP");
    expect(s).toBe("\u2605");
  });

  test("method returning a string (Unicode SMP)", () => {
    const s = java.callStaticMethodSync("Test", "getUnicodeSMP");
    // The below string is U+1F596, represented as surrogate pairs
    expect(s).toBe("\uD83D\uDD96");
  });

  test("method returning a string (NULL char)", () => {
    const s = java.callStaticMethodSync("Test", "getUnicodeNull");
    expect(s).toBe("\0");
  });

  test("method taking a byte", () => {
    const b = java.newByte(1);
    expect(b.getClassSync().getNameSync()).toBe('java.lang.Byte');
    expect(b.toStringSync()).toBe('1');
    const r = java.callStaticMethodSync("Test", "staticByte", b);
    expect(r).toBe(1);
  });

  test("method taking a short", () => {
    const s = java.newShort(1);
    expect(s.getClassSync().getNameSync()).toBe('java.lang.Short');
    expect(s.toStringSync()).toBe('1');
    const r = java.callStaticMethodSync("Test", "staticShort", s);
    expect(r).toBe(1);
  });

  test("method taking a double", () => {
    const s = java.newDouble(3.14);
    expect(s.getClassSync().getNameSync()).toBe('java.lang.Double');
    expect(s.toStringSync()).toBe('3.14');
    const r = java.callStaticMethodSync("Test", "staticDouble", s);
    expect(Math.abs(r - 3.14) < 0.0001, r + " != 3.14").toBeTruthy();
  });

  test("method taking a float", () => {
    const s = java.newFloat(3.14);
    expect(s.getClassSync().getNameSync()).toBe('java.lang.Float');
    expect(s.toStringSync()).toBe('3.14');
    const r = java.callStaticMethodSync("Test", "staticFloat", s);
    expect(Math.abs(r - 3.14) < 0.0001, r + " != 3.14").toBeTruthy();
  });

  test("method taking a long", () => {
    const l = java.newLong(1);
    expect(l.getClassSync().getNameSync()).toBe('java.lang.Long');
    expect(l.toStringSync()).toBe('1');
    const r = java.callStaticMethodSync("Test", "staticLong", l);
    expect(r).toBe(1);
  });

  test("method taking a char (number)", () => {
    const ch = java.newChar(97); // 'a'
    expect(ch.getClassSync().getNameSync()).toBe('java.lang.Character');
    expect(ch.toStringSync()).toBe('a');
    const r = java.callStaticMethodSync("Test", "staticChar", ch);
    expect(r).toBe(97);
  });

  test("method taking a char (string)", () => {
    const ch = java.newChar('a');
    expect(ch.getClassSync().getNameSync()).toBe('java.lang.Character');
    expect(ch.toStringSync()).toBe('a');
    const r = java.callStaticMethodSync("Test", "staticChar", ch);
    expect(r).toBe(97);
  });

  test("method taking a string (Unicode BMP)", () => {
    const s = "\u2605";
    const r = java.callStaticMethodSync("Test", "staticString", s);
    expect(r).toBe(s);
  });

  test("method taking a string (Unicode SMP)", () => {
    // The below string is U+1F596, represented as surrogate pairs
    const s = "\uD83D\uDD96";
    const r = java.callStaticMethodSync("Test", "staticString", s);
    expect(r).toBe(s);
  });

  test("method taking a string (with null char)", () => {
    const s = "\0";
    const r = java.callStaticMethodSync("Test", "staticString", s);
    expect(r).toBe(s);
  });

  test("new boolean array object", () => {
    const booleanArray = java.newArray("java.lang.Boolean", [true, false]);
    const r = java.callStaticMethodSync("Test", "staticBooleanArray", booleanArray);
    expect(r.length).toBe(2);
    expect(r[0]).toBe(true);
    expect(r[1]).toBe(false);
  });

  test("new byte array object", () => {
    const byteArray = java.newArray("byte", [1, 2, 3]);
    expect(byteArray.length).toBe(3);
    expect(byteArray[0]).toBe(1);
    expect(byteArray[1]).toBe(2);
    expect(byteArray[2]).toBe(3);
  });

  test("new boolean array", () => {
    const booleanArray = java.newArray("boolean", [true, false]);
    const r = java.callStaticMethodSync("Test", "staticBooleanArray", booleanArray);
    expect(r.length).toBe(2);
    expect(r[0]).toBe(true);
    expect(r[1]).toBe(false);
  });

  test("new int array", () => {
    const intArray = java.newArray("int", [1, 2]);
    const r = java.callStaticMethodSync("Test", "staticIntArray", intArray);
    expect(r.length).toBe(2);
    expect(r[0]).toBe(1);
    expect(r[1]).toBe(2);
  });

  test("new double array", () => {
    const doubleArray = java.newArray("double", [1.2, 4]);
    const r = java.callStaticMethodSync("Test", "staticDoubleArray", doubleArray);
    expect(r.length).toBe(2);
    expect(r[0]).toBe(1.2);
    expect(r[1]).toBe(4);
  });

  test("new short array objects", () => {
    const shortArray = java.newArray("java.lang.Short", [1, 2].map(function (c) { return java.newShort(c); }));
    const r = java.callStaticMethodSync("Test", "staticShortArray", shortArray);
    expect(r.length).toBe(2);
    expect(r[0]).toBe(1);
    expect(r[1]).toBe(2);
  });

  test("new short array", () => {
    const shortArray = java.newArray("short", [1, 2]);
    const r = java.callStaticMethodSync("Test", "staticShortArray", shortArray);
    expect(r.length).toBe(2);
    expect(r[0]).toBe(1);
    expect(r[1]).toBe(2);
  });
});

