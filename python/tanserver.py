# -*- coding: utf-8 -*-

# Copyright (C) tanserver.org
# Copyright (C) Chen Daye

import json
from ctypes import *

json._default_encoder.ensure_ascii  = False
json._default_encoder.key_separator = ':'

# shm_open()
CDLL('librt.so', mode=RTLD_GLOBAL)

# PostgreSQL C Client
CDLL('libpq.so', mode=RTLD_GLOBAL)

# libconfuse
CDLL('/usr/local/tanserver/lib/libconfuse.so',
     mode=RTLD_GLOBAL)

# JsonCpp
CDLL('/usr/local/tanserver/lib/libjsoncpp.so',
     mode=RTLD_GLOBAL)

# Currently supports pg_query(), json_append_status()
core = CDLL('/usr/local/tanserver/lib/libcore.so')

# Set argtypes
core.pg_query.argtypes           = [c_char_p, c_char_p]
core.json_append_status.argtypes = [c_char_p, c_char_p, c_char_p]

# Set restype
core.pg_query.restype           = c_char_p
core.json_append_status.restype = c_char_p

def pg_query(hostaddr, query, *args):
  """Query database and get the first field of the first row.

  ``hostaddr``: Database server IP address (/etc/tanserver.conf)

  ``query``: SQL statement, use $1, $2, $3... to replace variable arguments

  ``args``: Variable arguments, only supports string

  ``return``: The first field of the first row or an empty string.

  ``exception``: When the query fails, an exception will be thrown
  and the reason will be automatically written to the log
  (/var/log/tanserver/error.log).

  """

  args_encoded = ()

  for ele in args:
    args_encoded = (*args_encoded, (str(ele)).encode())

  res = core.pg_query(hostaddr.encode(), query.encode(), *args_encoded)

  # pg_query() returned NULL in C.
  if res == None:
      raise Exception("Query failed.")

  return res.decode()

def json_append_status(json_string, status_code, message):
  """Append status code and message to a JSON string.

  ``json_string``: The JSON string to be processed

  ``status_code``: The specified status code

  ``message``: The specified message

  ``return``: For example: {"status":200,"message":"OK","result":{$json_string}}

  """
  return (core.json_append_status(json_string.encode(), status_code.encode(), message.encode())).decode()
