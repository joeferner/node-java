#!/bin/bash -e

# JAVA_VERSION=1.6
# JAVAC_OPTS="-source ${JAVA_VERSION} -target ${JAVA_VERSION}"

# javac defaults -source <version> and -target <version>
# to it's version.
# We support three versions of javac: 1.6, 1.7, 1.8.
# We want the following mappings:
# javac    source     target
#   1.6       1.6        1.6
#   1.7       1.6        1.6
#   1.8       1.8        1.8
# Note that the default is only wrong for javac 1.7.
# We'll have to fix this, but for now we do the hack
# of letting javac 1.7 generate with source and target equal to 1.7

JAVAC_OPTS=""

cd test
javac ${JAVAC_OPTS} *.java

cd ../src-java/node
javac ${JAVAC_OPTS} *.java

cd ../../
javah -classpath src-java -d ./src node.NodeDynamicProxyClass
