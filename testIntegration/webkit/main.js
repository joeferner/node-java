const java = require("../../");

export function runJavaMethod() {
  return java.callStaticMethodSync("java.lang.System", "getProperty", "java.version");
}
