
var java = require("./");
java.options.push("-Djava.awt.headless=true");
java.classpath.push("test/");
java.classpath.push("test/commons-lang3-3.1.jar");

module.exports.java = java;
