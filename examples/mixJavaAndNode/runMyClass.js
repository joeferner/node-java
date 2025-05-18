#!/usr/bin/env node

const java = require("../../");
java.classpath.push("./src");

const MyClass = java.import("com.nearinfinity.nodeJava.MyClass");

const result = MyClass.addNumbersSync(1, 2);
console.log(result);
