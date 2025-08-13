import { AsyncOptions, Java, JavaError } from "./java";
import findRoot from "find-root";

const root = findRoot(__dirname);
let java: Promise<Java> | undefined;

export interface GetJavaOptions {
  beforeInit?: (java: Java) => void | Promise<void>;
}

export async function getJava(asyncOptions?: AsyncOptions | null, options?: GetJavaOptions): Promise<Java> {
  if (java) {
    return java;
  }
  java = _getJava(asyncOptions, options);
  return java;
}

async function _getJava(asyncOptions?: AsyncOptions | null, options?: GetJavaOptions): Promise<Java> {
  const java = (await import(root)).default as Java;

  java.options.push("-Djava.awt.headless=true");
  //java.options.push('-agentlib:jdwp=transport=dt_socket,server=y,suspend=y,address=5005');

  java.classpath.push("test/");
  java.classpath.push("test/commons-lang3-3.18.0.jar");

  if (asyncOptions !== null) {
    java.asyncOptions = asyncOptions ?? {
      syncSuffix: "Sync",
      asyncSuffix: "",
      promiseSuffix: "Promise",
    };
  }

  await options?.beforeInit?.(java);

  // force initialization
  java.import("java.util.ArrayList");

  return java;
}

export function expectJavaError(error: unknown): asserts error is JavaError {}
