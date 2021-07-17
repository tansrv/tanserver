#! /bin/bash

# 1. libconfuse.so
# 2. libjsoncpp.so
# 3. libcore.so
# 4. libconfuse.so + libjsoncpp.so + libcore.so = tanserver (exe)

cd ../src/lib/confuse/build && cmake .. && make && make install && \
cd ../../jsoncpp/build/     && cmake .. && make && make install && \
cd ../../../core/build/     && cmake .. && make && make install && \
cd ../../build/             && cmake .. && make && make install && \

# Get Python APIs
pip3 install tanserver

if [ $? -ne 0 ]; then
  echo -e "\ntanserver: install failed\n"
fi
