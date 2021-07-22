#! /bin/bash

yum -y install libarchive gcc gcc-c++ kernel-devel make cmake3 python3 python3-devel python3-pip openssl-devel postgresql-devel && \
cd ../src/lib/confuse/build && cmake3 .. && make && make install                                                                && \
cd ../../jsoncpp/build/     && cmake3 .. && make && make install                                                                && \
cd ../../../core/build/     && cmake3 .. && make && make install                                                                && \
cd ../../build/             && cmake3 .. && make && make install                                                                && \
pip3 install tanserver

if [ $? -ne 0 ]; then
  echo -e "\ntanserver: install failed\n"
fi
