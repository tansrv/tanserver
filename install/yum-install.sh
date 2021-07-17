#! /bin/bash

yum -y install libarchive gcc gcc-c++ kernel-devel make cmake python2-devel python3-pip openssl-devel postgresql-devel && \
chmod +x install.sh                                                                                                    && \
./install.sh
