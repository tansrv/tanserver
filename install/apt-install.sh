#! /bin/bash

apt-get update                                                                                && \
apt-get -y install build-essential cmake python3 python3-dev python3-pip libssl-dev libpq-dev && \
cd ../src/lib/confuse/build && cmake .. && make && make install                               && \
cd ../../jsoncpp/build/     && cmake .. && make && make install                               && \
cd ../../../core/build/     && cmake .. && make && make install                               && \
cd ../../build/             && cmake .. && make && make install                               && \
pip3 install tanserver

if [ $? -ne 0 ]; then
  echo -e "\ntanserver: install failed\n"
fi
