#!/bin/sh

if [ -z "${JAVA_HOME}" ]; then
    export JAVA_HOME=`/usr/libexec/java_home -v 11`
fi

echo "Using `javac -version`."
set -x

javac -source 1.7 -target 1.7 src-java/node/*.java
javac -source 1.7 -target 1.7 test/*.java
javac -source 1.8 -target 1.8 test8/*.java
javah -classpath src-java -d ./src node.NodeDynamicProxyClass
