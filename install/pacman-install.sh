#! /bin/bash

pacman -Sy && \
pacman -S gcc make cmake openssl postgresql-libs --noconfirm --needed && \
chmod +x install.sh                              && \
./install.sh
