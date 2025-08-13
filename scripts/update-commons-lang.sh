#!/bin/sh
set -eu
SCRIPT_DIR=$(dirname $(realpath $0))

cd "${SCRIPT_DIR}/commons-lang"
mvn package
cp "${SCRIPT_DIR}/commons-lang/target/commons-lang-1.jar" "${SCRIPT_DIR}/../src-java/commons-lang3-node-java.jar"

echo "complete!"
