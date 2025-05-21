#!/bin/sh
set -eu

SCRIPT_DIR=$(dirname "$0")
cd "${SCRIPT_DIR}/.."

if [ -d build/commons-lang ]; then
  cd build/commons-lang
  git pull
else
  mkdir -p build
  cd build
  git clone --depth 1 git@github.com:apache/commons-lang.git
  cd commons-lang
fi

mvn clean compile package -DskipTests
java -jar ../../src-java/jarjar-1.4.jar process ../../src-java/commons-lang.jarjar.rules target/commons-lang3*-SNAPSHOT.jar ../../src-java/commons-lang3-node-java.jar

echo "complete!"
