#! /bin/bash

apt-get update                                                              && \
apt-get -y install build-essential cmake python2.7-dev libssl-dev libpq-dev && \
chmod +x install.sh                                                         && \
./install.sh
