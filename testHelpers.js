export const java = require("./");
java.options.push("-Djava.awt.headless=true");
//java.options.push('-agentlib:jdwp=transport=dt_socket,server=y,suspend=y,address=5005');

java.classpath.push("test/");
java.classpath.push("test/commons-lang3-3.1.jar");

export function getJava(asyncOptions) {
  java.asyncOptions = asyncOptions ?? {
    syncSuffix: "Sync",
    asyncSuffix: "",
    promiseSuffix: "Promise",
    promisify: require("when/node").lift, // https://github.com/cujojs/when
  };

  // force initialization
  java.import("java.util.ArrayList");

  return java;
}
