
var bindings = require("../build/Release/nodejavabridge_bindings");

var java = module.exports = new bindings.Java();
java.classpath.push(__dirname + "/../commons-lang3-node-java.jar");
