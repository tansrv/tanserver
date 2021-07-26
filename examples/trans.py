# docs: tanserver.org

from tanserver import *

# API: transaction
def transaction(json_obj):
    try:
        pg_query('127.0.0.1', 'begin')

        pg_query('127.0.0.1',
                 'update accounts set num = num - $1 where name = $2',
                 '10', 'Jack')

        pg_query('127.0.0.1',
                 'update accounts set num = num + $1 where name = $2',
                 '10', 'Mike')

        pg_query('127.0.0.1', 'commit')

        # Send "OK"
        return 'OK'
    except:
        pg_query('127.0.0.1', 'rollback')
        return 'Query failed'
