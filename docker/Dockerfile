FROM ubuntu:16.04

RUN apt-get update -y
RUN apt-get install -y curl wget tar git python build-essential

# Java
RUN \
  wget https://javadl.oracle.com/webapps/download/GetFile/1.8.0_261-b12/a4634525489241b9a9e1aa73d9e118e6/linux-i586/jdk-8u261-linux-x64.tar.gz -O /opt/jdk-8u261-linux-x64.tar.gz \
  && tar -xzf /opt/jdk-8u261-linux-x64.tar.gz -C /opt \
  && rm /opt/jdk-8u261-linux-x64.tar.gz \
  && ln -s /opt/jdk1.8.0_261 /opt/jdk
ENV PATH $PATH:/opt/jdk/bin
ENV JAVA_HOME /opt/jdk
ENV _JAVA_OPTIONS -Djava.net.preferIPv4Stack=true

# Node
RUN \
  wget -O /opt/node-v11.2.0-linux-x64.tar.gz https://nodejs.org/dist/v11.2.0/node-v11.2.0-linux-x64.tar.gz \
  && tar -xzf /opt/node-v11.2.0-linux-x64.tar.gz -C /opt \
  && rm /opt/node-v11.2.0-linux-x64.tar.gz \
  && ln -s /opt/node-v11.2.0-linux-x64 /opt/node
ENV PATH $PATH:/opt/node/bin

ENV USER=root