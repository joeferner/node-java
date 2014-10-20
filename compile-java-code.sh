#!/bin/bash -e

JAVA_VERSION=1.6
JAVAC_OPTS="-source ${JAVA_VERSION} -target ${JAVA_VERSION} -bootclasspath /opt/jdk1.6.0_45/jre/lib/rt.jar"

cd test
javac ${JAVAC_OPTS} *.java

cd ../src-java/node
javac ${JAVAC_OPTS} *.java

cd ../../
javah -classpath src-java -d ./src node.NodeDynamicProxyClass
