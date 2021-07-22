#! /bin/bash

pacman -Sy                                                                               && \
pacman -S gcc make cmake openssl python3 python-pip postgresql-libs --noconfirm --needed && \
cd ../src/lib/confuse/build && cmake .. && make && make install                          && \
cd ../../jsoncpp/build/     && cmake .. && make && make install                          && \
cd ../../../core/build/     && cmake .. && make && make install                          && \
cd ../../build/             && cmake .. && make && make install                          && \
pip3 install tanserver

if [ $? -ne 0 ]; then
  echo -e "\ntanserver: install failed\n"
fi
