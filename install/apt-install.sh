#! /bin/bash

apt-get update                                                                                && \
apt-get -y install build-essential cmake python3 python3-dev python3-pip libssl-dev libpq-dev && \
chmod +x install.sh                                                                           && \
./install.sh
