FROM mcr.microsoft.com/devcontainers/javascript-node:22

# need to run update and install at the same time to prevent stale update layer
RUN apt-get update && apt-get install -y gdb clang-format openjdk-17-jdk maven
