#! /bin/bash

pacman -S gcc make cmake openssl postgresql-libs && \
chmod +x install.sh                              && \
./install.sh
