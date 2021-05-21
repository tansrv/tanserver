#! /bin/bash

apt-get update                           && \
apt-get -y install build-essential cmake && \
apt-get -y install libssl-dev libpq-dev  && \
chmod +x install.sh                      && \
./install.sh
