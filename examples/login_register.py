# docs: tanserver.org

import json
from tanserver import *

# API: login_register
# Let users log in or register.
#
# users (table):
# id  password  age
# 0   mypwd     21
# 1   mypwd2    8
def login_register(json_obj):
    # Get id and password from JSON string sent by the client.
    id  = json_obj.get('id')
    pwd = json_obj.get('password')

    if id == None or pwd == None:
        return 'missing id or password'

    try:
        # Check if the user exists.
        # If the user exists, convert the row to a JSON string and send it to the client.
        res = pg_query('127.0.0.1',
                       'select row_to_json(users) from users where id = $1 and password = $2',
                       id, pwd)

        # If 'res' is an empty string, it means there is no such user.
        if res == '':
            # Complete registration.
            pg_query('127.0.0.1',
                     'insert into users values($1, $2, $3)',
                     id, pwd, '16')

            data = {}
            data['msg'] = 'registration successful'

            # Send {"msg": "registration successful"}
            return json.dumps(data)

        # Send JSON string, like: {"id": 0, "password": "mypwd", "age": 21}
        return res
    except:
        return 'Query failed'
