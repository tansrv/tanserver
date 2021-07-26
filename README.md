<div align=center>
    <img src="tanserver.png">
</div>

<div align=center>
    <img src="https://img.shields.io/badge/os-linux-orange">
    <img src="https://img.shields.io/badge/license-BSD-blue">
    <img src="https://img.shields.io/badge/docs-latest-brightgreen">
</div>

# Tanserver

Tanserver is a high performance Linux server framework that allows you to easily and efficiently develop server APIs in python for `Android` / `iOS` / `desktop` / `web` apps.  
With this framework you can build a stable, secure and customizable client-server communication and also manage data thanks to [PostgreSQL](https://www.postgresql.org/) integration.

## Documentation

* [Install Tanserver](https://tanserver.org/en/install/)
* [Tanserver documentation](https://www.tanserver.org)

## How it works
Tanserver receives requests as JSON string from the client using a custom protocol that improves response time.  
JSON string is composed by an header that contains API name and a body that contains parameters passed to API.
After completing the computation and eventually querying the database, Tanserver sends a response to the client as JSON data.

## Features

- **Security** ：Encrypted based on SSL protocol.

- **Hot reload** ：Help you quickly add APIs or fix bugs.

- **Log shipping** ：Support log shipping.

## Client connectors

You need a connector to connect your client app to tanserver, currently `Java`, `Javascript` and `Swift` clients are supported. (`Dart` / `C++` will be supported soon.)

1. [Java](https://github.com/tansrv/tanserver-connectors/tree/main/java)

2. [Swift](https://github.com/tansrv/tanserver-connectors/tree/main/swift)

3. [Javascript](https://github.com/tansrv/tanserver-connectors/tree/main/javascript)

## API example

API: `get_items`

Convert items table data into JSON string and send it to the client.

`items`
| id   | name     | price |
| ---- | ----     | ----  |
| 0    | book     | 5     |
| 1    | keyboard | 200   |

```python
from tanserver import *

def get_items(json_obj):
    try:
        res = pg_query('127.0.0.1', # Your PostgreSQL server IP address
                       'select array_to_json(array_agg(row_to_json(items))) from items')

        return json_append_status(res, 0, 'OK')
    except:
        return 'Query failed'
```

When the client calls `getJSON("get_items", "{}")`:

![get_items](get_items.gif)

---

Copyright (C) tanserver.org
