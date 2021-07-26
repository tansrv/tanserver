# docs: tanserver.org

from tanserver import *

# API: append_status
# Append status code and message to a JSON string.
def append_status(json_obj):
    str = '{\"id\":0}'

    # Send {"status": 200, "message": "ok", "result": {"id": 0}}
    return json_append_status(str, 200, 'ok')
