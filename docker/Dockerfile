FROM ubuntu:16.04

RUN apt-get update -y
RUN apt-get install -y curl wget tar git python build-essential

# Java
RUN \
  wget --continue --no-check-certificate -O /opt/jdk-8u191-linux-x64.tar.gz --header "Cookie: oraclelicense=a" http://download.oracle.com/otn-pub/java/jdk/8u191-b12/2787e4a523244c269598db4e85c51e0c/jdk-8u191-linux-x64.tar.gz \
  && tar -xzf /opt/jdk-8u191-linux-x64.tar.gz -C /opt \
  && rm /opt/jdk-8u191-linux-x64.tar.gz \
  && ln -s /opt/jdk1.8.0_191 /opt/jdk
ENV PATH $PATH:/opt/jdk/bin
ENV JAVA_HOME /opt/jdk
ENV _JAVA_OPTIONS -Djava.net.preferIPv4Stack=true

# Node
RUN \
  wget -O /opt/node-v11.2.0-linux-x64.tar.gz http://nodejs.org/dist/latest/node-v11.2.0-linux-x64.tar.gz \
  && tar -xzf /opt/node-v11.2.0-linux-x64.tar.gz -C /opt \
  && rm /opt/node-v11.2.0-linux-x64.tar.gz \
  && ln -s /opt/node-v11.2.0-linux-x64 /opt/node
ENV PATH $PATH:/opt/node/bin

ENV USER=root