#! /bin/bash

pacman -Sy                                                                               && \
pacman -S gcc make cmake openssl python2 python-pip postgresql-libs --noconfirm --needed && \
chmod +x install.sh                                                                      && \
./install.sh
