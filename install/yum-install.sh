#! /bin/bash

yum -y install libarchive gcc gcc-c++ kernel-devel make cmake && \
yum -y install openssl-devel postgresql-devel                 && \
chmod +x install.sh                                           && \
./install.sh
