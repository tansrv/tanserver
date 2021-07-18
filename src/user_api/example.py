# docs: tanserver.org


import json
import tanserver


# API: hello_world
def hello_world(json_obj):
    # Send "hello, world!" 
    return 'hello, world!'


# API: hello_tanserver
def hello_tanserver(json_obj):
    # Get username
    user = json_obj.get('username')
    if user == None:
        return 'missing username'

    data = {}

    if user == 'tanserver': # {"hello": "tanserver"}
        data['hello'] = 'tanserver'
    else:                   # {"error": "unknown username"}
        data['error'] = 'unknown username'

    return json.dumps(data)


# API: append_status
def append_status(json_obj):
    str = '{\"id\":0}'

    # Send {"status": 200, "message": "ok", "result": {"id": 0}}
    return tanserver.json_append_status(str, '200', 'ok')


# API: login_register
def login_register(json_obj):
    id  = json_obj.get('id')
    pwd = json_obj.get('password')

    if id == None or pwd == None:
        return 'missing id or password'

    try:
        res = tanserver.pg_query('127.0.0.1',
                                 'select row_to_json(users) from users where id = $1 and password = $2',
                                 id, pwd)

        # If 'res' is an empty string, it means there is no such user.
        if res == '':
            # Complete registration.
            tanserver.pg_query('127.0.0.1',
                               'insert into users values($1, $2, $3)'
                               id, pwd, '16')

            data = {}
            data['msg'] = 'registration successful'

            # Send {"msg": "registration successful"}
            return json.dumps(data)
        else:
            return res
    except:
        return 'Query failed'


def get_items(json_obj):
    try:
        res = tanserver.pg_query('127.0.0.1',
                                 'select array_to_json(array_agg(row_to_json(items))) from items')

        return tanserver.json_append_status(res, '0', 'OK')
    except:
        return 'Query failed'


def transaction(json_obj):
    try:
        tanserver.pg_query('127.0.0.1', 'begin')

        tanserver.pg_query('127.0.0.1',
                           'update accounts set num = num - $1 where name = $2',
                           '10', 'Jack')

        tanserver.pg_query('127.0.0.1',
                           'update accounts set num = num + $1 where name = $2',
                           '10', 'Mike')

        tanserver.pg_query('127.0.0.1', 'commit')

        # Send "OK"
        return 'OK'
    except:
        return 'Query failed'
