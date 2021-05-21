cmake_minimum_required(VERSION 2.8)

project(jsoncpp)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)

set(INSTALL_PATH /usr/local/tanserver)
set(LIB ${INSTALL_PATH}/lib)

aux_source_directory(. SRCS)
add_library(jsoncpp SHARED ${SRCS})

install(TARGETS jsoncpp LIBRARY DESTINATION ${LIB})
install(FILES json.h DESTINATION include)
