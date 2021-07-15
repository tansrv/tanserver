#! /bin/bash

apt-get update                                                && \
apt-get -y install build-essential cmake libssl-dev libpq-dev && \
chmod +x install.sh                                           && \
./install.sh
