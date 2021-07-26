# docs: tanserver.org

from tanserver import *

# API: get_items
# Convert items table data into JSON string and send it to the client.
#
# items (table):
# id  name      price
# 0   book	    5
# 1   keyboard  200
def get_items(json_obj):
    try:
        res = pg_query('127.0.0.1',
                       'select array_to_json(array_agg(row_to_json(items))) from items')
        # Send
        # {
        #     "status":0,
        #     "message":"OK",
        #     "result":[
        #         {
        #             "id":0,
        #             "name":"book",
        #             "price":5
        #         },
        #         {
        #             "id":1,
        #             "name":"keyboard",
        #             "price":200
        #         }
        #     ]
        # }
        return json_append_status(res, 0, 'OK')

        # Exclude price, send
        # {
        #     "status":200,
        #     "message":"OK",
        #     "result":[
        #         {
        #             "id":0,
        #             "name":"book",
        #         },
        #         {
        #             "id":1,
        #             "name":"keyboard",
        #         }
        #     ]
        # }

        # res = pg_query('127.0.0.1',
        #                'select array_to_json(array_agg(json_build_object('id', id, 'name', name))) from items')

        # return json_append_status(res, 200, 'OK')
    except:
        return 'Query failed'
