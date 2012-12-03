#!/usr/bin/env node

var java = require("../../");
java.classpath.push("./src");

var MyClass = java.import("com.nearinfinity.nodeJava.MyClass");

var result = MyClass.addNumbersSync(1, 2);
console.log(result);


