#! /bin/bash

cd ../src/lib/confuse/build    && cmake .. && make && make install && \
cd ../../jsoncpp/build/        && cmake .. && make && make install && \
cd ../../../core/build/        && cmake .. && make && make install && \
cd ../../build/                && cmake .. && make && make install && \
cd /usr/local/tanserver/build/ && cmake .  && make && make install

if [ $? -ne 0 ]; then
  echo -e "\ntanserver: install failed\n"
fi
