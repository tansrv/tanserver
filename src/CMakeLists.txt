cmake_minimum_required(VERSION 2.8)

project(tanserver)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)

set(INSTALL_PATH /usr/local/tanserver)
set(LIB ${INSTALL_PATH}/lib)
set(BIN ${INSTALL_PATH}/sbin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN})

link_directories(${LIB})

add_executable(tanserver build/null.c)

target_link_libraries(tanserver libcore.so libconfuse.so libjsoncpp.so python2.7 pthread rt crypto ssl pq)

install(FILES ../conf/tanserver.conf DESTINATION /etc)
install(DIRECTORY user_api DESTINATION ${INSTALL_PATH})
