# docs: tanserver.org

import json
from tanserver import *

# API: hello_tanserver
def hello_tanserver(json_obj):
    # Get username from JSON string sent by the client.
    user = json_obj.get('username')
    if user == None:
        return 'missing username'

    data = {}

    # If username is tanserver then send {"hello": "tanserver"} to the client.
    # Otherwise send {"error": "unknown username"}.
    if user == 'tanserver':
        data['hello'] = 'tanserver'
    else:
        data['error'] = 'unknown username'

    return json.dumps(data)
