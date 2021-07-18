#! /bin/bash

apt-get update                                                                           && \
apt-get -y install wget build-essential cmake python2 python2.7-dev libssl-dev libpq-dev && \
chmod +x install.sh                                                                      && \
./install.sh
