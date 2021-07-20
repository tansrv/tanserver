#! /bin/bash

# 1. libconfuse.so
# 2. libjsoncpp.so
# 3. libcore.so
# 4. libconfuse.so + libjsoncpp.so + libcore.so = tanserver (exe)
# 5. Get Tanserver Python APIs

cd ../src/lib/confuse/build && cmake .. && make && make install && \
cd ../../jsoncpp/build/     && cmake .. && make && make install && \
cd ../../../core/build/     && cmake .. && make && make install && \
cd ../../build/             && cmake .. && make && make install && \
pip3 install tanserver

if [ $? -ne 0 ]; then
  echo -e "\ntanserver: install failed\n"
fi
