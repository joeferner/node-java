#!/bin/bash -e

# This script must be run on a Mac due to its reliance on /usr/libexec/java_home
# to find a JDK 1.8 installation. Note that this script will work correctly on
# a mac with JDK 1.8 installed, even if JAVA_HOME currently points to a 1.7
# or earlier JDK.
# This script is run manually by maintainers of this project, who add the
# the generated .class files to source control.

JAVA_VERSION=1.8
JDK8_HOME=$(/usr/libexec/java_home -v ${JAVA_VERSION})

cd test8
${JDK8_HOME}/bin/javac -source ${JAVA_VERSION} *.java
