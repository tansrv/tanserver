cmake_minimum_required(VERSION 2.8)

project(user_api)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)

set(INSTALL_PATH /usr/local/tanserver)
set(LIB ${INSTALL_PATH}/lib)

aux_source_directory(../user_api SRCS)
add_library(user_api SHARED ${SRCS})

install(TARGETS user_api LIBRARY DESTINATION ${LIB})
