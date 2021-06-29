<div align=center>
    <img src="tanserver.png">
</div>

<div align=center>
    <img src="https://img.shields.io/badge/os-linux-orange">
    <img src="https://img.shields.io/badge/license-BSD-blue">
    <img src="https://img.shields.io/badge/docs-latest-brightgreen">
</div>

# Tanserver

Tanserver is a server framework built for APIs.

It receives JSON string from the client and returns JSON string after querying the database.

You can apply Tanserver to your `Android` / `iOS` / `desktop` apps.

## Documentation

* [Install Tanserver](https://tanserver.org/en/install/)
* [Tanserver documentation](https://www.tanserver.org)

## Features

- **Security** ：Encrypted based on SSL protocol.

- **Hot reload** ：Help you quickly add APIs or fix bugs.

- **Log shipping** ：Support log shipping.

## Connector

1. [Java](https://github.com/tansrv/tanserver-connector-java)

2. [Swift](https://github.com/tansrv/tanserver-connector-swift)

Currently `Java` and `Swift` clients are supported. (`Dart` / `C++` will be supported in the future.)

## Example

API: `get_items`

Convert items table data into JSON string and send it to the client.

`items`
| id   | name     | price |
| ---- | ----     | ----  |
| 0    | book     | 5     |
| 1    | keyboard | 200   |

```cpp
string
get_items(const Json::Value &value_recvd)
{
    try {
        string  json_string = pg_query("8.8.8.8", /* Your PostgreSQL server IP address  */
                                       "select array_to_json(array_agg(row_to_json(items))) from items;",
                                       NULL);

        json_append_status(json_string, 0, "OK");
        return json_string;
    } catch (...) {
        throw "error";
    }
}
```

When the client calls `getJSON("get_items", "{}")`:

![get_items](get_items.gif)

---

Copyright (C) tanserver.org
