var java = require('../../');

function runJavaMethod() {
  return java.callStaticMethodSync("java.lang.System", "getProperty", "java.version");
}