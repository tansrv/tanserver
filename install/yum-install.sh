#! /bin/bash

yum -y install libarchive gcc gcc-c++ kernel-devel make cmake python3 python3-devel python3-pip openssl-devel postgresql-devel && \
chmod +x install.sh                                                                                                            && \
./install.sh
