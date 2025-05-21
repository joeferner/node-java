#!/bin/sh
set -eu

SCRIPT_DIR=$(dirname "$0")
cd "${SCRIPT_DIR}/.."

if [ -z "${JAVA_HOME:-}" ] && [ -e "/usr/libexec/java_home" ]; then
    export JAVA_HOME=$(/usr/libexec/java_home -v 11)
fi

echo "Using $(javac -version)."
set -x

javac -source 1.8 -target 1.8 src-java/node/*.java
javac -source 1.8 -target 1.8 test/*.java
javac -classpath src-java -h ./src-cpp src-java/node/NodeDynamicProxyClass.java
echo "complete!"
