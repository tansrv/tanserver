cmake_minimum_required(VERSION 2.8)

project(core)

if(EXISTS /usr/include/libpq-fe.h)
  option(TAN_HAVE_REDHAT_PG_HEADER "#include <libpq-fe.h>" ON)
endif()

configure_file(
  ../../conf/config.h.in
  ${PROJECT_SOURCE_DIR}/config.h
)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)

set(INSTALL_PATH /usr/local/tanserver)
set(LIB ${INSTALL_PATH}/lib)
set(INCLUDE /usr/local/include/tanserver)

aux_source_directory(. SRCS)
add_library(core SHARED ${SRCS})

install(TARGETS core LIBRARY DESTINATION ${LIB})
install(FILES ../../misc/tanserver PERMISSIONS OWNER_EXECUTE DESTINATION /etc/init.d)
