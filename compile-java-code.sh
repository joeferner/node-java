#!/bin/bash -e

JAVA_VERSION=1.8
JAVAC_OPTS="-source ${JAVA_VERSION} -target ${JAVA_VERSION}"  -bootclasspath ${JAVA_HOME}/jre/lib/rt.jar

cd test
javac ${JAVAC_OPTS} *.java

cd ../src-java/node
javac ${JAVAC_OPTS} *.java

cd ../../
javah -classpath src-java -d ./src node.NodeDynamicProxyClass
