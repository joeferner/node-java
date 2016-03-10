#!/bin/bash -e

JAVA_VERSION=1.7

if [ -x /usr/libexec/java_home ]; then
  # On MacOS we can do this to choose the Java 1.7 JDK
  # and then let the JDK default -source and -target to 1.7
  JAVA_HOME=$(/usr/libexec/java_home -v ${JAVA_VERSION})

  cd test
  ${JAVA_HOME}/bin/javac *.java

  cd ../src-java/node
  ${JAVA_HOME}/bin/javac *.java

  cd ../../
  ${JAVA_HOME}/bin/javah -classpath src-java -d ./src node.NodeDynamicProxyClass
elif [ -n "$1" ]; then
  JAVA_HOME=$1
  JAVAC_OPTS="-source ${JAVA_VERSION} -target ${JAVA_VERSION} -bootclasspath ${JAVA_HOME}/jre/lib/rt.jar"

  cd test
  ${JAVA_HOME}/bin/javac ${JAVAC_OPTS} *.java

  cd ../src-java/node
  ${JAVA_HOME}/bin/javac ${JAVAC_OPTS} *.java

  cd ../../
  ${JAVA_HOME}/bin/javah -classpath src-java -d ./src node.NodeDynamicProxyClass
else
  JAVAC_OPTS="-source ${JAVA_VERSION} -target ${JAVA_VERSION} -bootclasspath /opt/jdk/jre/lib/rt.jar"

  cd test
  javac ${JAVAC_OPTS} *.java

  cd ../src-java/node
  javac ${JAVAC_OPTS} *.java

  cd ../../
  javah -classpath src-java -d ./src node.NodeDynamicProxyClass
fi


