cmake_minimum_required(VERSION 2.8)

project(confuse)

set(INSTALL_PATH /usr/local/tanserver)
set(LIB ${INSTALL_PATH}/lib)

aux_source_directory(. SRCS)
add_library(confuse SHARED ${SRCS})

install(TARGETS confuse LIBRARY DESTINATION ${LIB})
install(FILES confuse.h DESTINATION include)
