
var java = require("./");
java.options.push("-Djava.awt.headless=true");
//java.options.push('-agentlib:jdwp=transport=dt_socket,server=y,suspend=y,address=5005');

java.classpath.push("test/");
java.classpath.push("test/commons-lang3-3.1.jar");
java.classpath.push("test8/");

module.exports.java = java;
