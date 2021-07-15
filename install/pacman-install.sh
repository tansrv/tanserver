#! /bin/bash

pacman -Sy                                                                    && \
pacman -S gcc make cmake python2 openssl postgresql-libs --noconfirm --needed && \
chmod +x install.sh                                                           && \
./install.sh
