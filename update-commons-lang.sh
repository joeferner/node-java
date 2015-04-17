#!/bin/bash -ex

if [ -d build/commons-lang ]; then
  cd build/commons-lang
  git pull
else
  mkdir -p build
  cd build
  git clone --depth 1 git@github.com:joeferner/commons-lang.git
  cd commons-lang
fi

mvn clean compile package -DskipTests
java -jar ../../jarjar-1.4.jar process ../../commons-lang.jarjar.rules target/commons-lang3*-SNAPSHOT.jar ../../commons-lang3-node-java.jar

