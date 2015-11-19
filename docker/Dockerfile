FROM ubuntu

RUN apt-get update -y
RUN apt-get install -y curl wget tar git python build-essential

# Java
RUN \
  wget --header "Cookie: oraclelicense=accept-securebackup-cookie" -O /opt/jdk-8u65-linux-x64.tar.gz http://download.oracle.com/otn-pub/java/jdk/8u65-b17/jdk-8u65-linux-x64.tar.gz \
  && tar -xzf /opt/jdk-8u65-linux-x64.tar.gz -C /opt \
  && rm /opt/jdk-8u65-linux-x64.tar.gz \
  && ln -s /opt/jdk1.8.0_65 /opt/jdk
ENV PATH $PATH:/opt/jdk/bin
ENV JAVA_HOME /opt/jdk
ENV _JAVA_OPTIONS -Djava.net.preferIPv4Stack=true

# Node
RUN \
  wget -O /opt/node-v4.2.2-linux-x64.tar.gz https://nodejs.org/dist/v4.2.2/node-v4.2.2-linux-x64.tar.gz \
  && tar -xzf /opt/node-v4.2.2-linux-x64.tar.gz -C /opt \
  && rm /opt/node-v4.2.2-linux-x64.tar.gz \
  && ln -s /opt/node-v4.2.2-linux-x64 /opt/node
ENV PATH $PATH:/opt/node/bin

ENV USER=root
