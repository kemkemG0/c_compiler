FROM ubuntu:latest
RUN apt-get update && apt-get install -y sudo gcc make git binutils libc6-dev
WORKDIR /workdir