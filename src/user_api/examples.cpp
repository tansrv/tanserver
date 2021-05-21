/*
 * Docs:     tanserver.org
 * Feedback: tanserver@outlook.com
 */


#include <tanserver/user.h>


extern "C" {


#if 0
/*
 * API 1: hello_world
 *
 * Send "hello, world" to the client
 */
string
hello_world(const Json::Value &value_recvd)
{
    /* Send "hello, world"  */
    return "hello, world";
}
#endif


/*
 * API 2: hello_tanserver
 *
 * 1. Get username from JSON string sent by the client
 *
 * 2. If username is tanserver then
 *    send {"hello": "tanserver"} to the client
 *
 * 3. Otherwise send {"error": "unknown username"}
 */
string
hello_tanserver(const Json::Value &value_recvd)
{
    /* Get username  */
    string  user = value_recvd["username"].asString();
    if (user.empty())
        return "missing username";

    Json::Value  value;

    if (user == "tanserver") {

        value["hello"] = "tanserver";

        /* Send {"hello": "tanserver"}  */
        return json_encode(value);
    }

    value["error"] = "unknown username";

    /* Send {"error": "unknown username"}  */
    return json_encode(value);
}


/*
 * API 3: append_status
 *
 * Append status code and message to JSON string
 */
string
append_status(const Json::Value &value_recvd)
{
    string  json_string = "{\"id\": 0}";

    /* json_string = {"status": 200, "message": "ok", "result": {"id": 0}}  */
    json_append_status(json_string, 200, "ok");

    /* 
     * json_string = {"status": -1, "message": "failed", "result": {"id": 0}}
     * json_append_status(json_string, -1, "failed");
     */

    return json_string;
}


/*
 * API 4: login_register
 *
 * 1. Get id and password from JSON string sent by the client
 *
 * 2. Check if the user exists
 *
 * 3. If the user exists, convert the row to a JSON string
 *    and send it to the client
 *
 * 4. Otherwise, complete the registration and
 *    send {"msg": "Registration successful"}
 *
 * users
 * +----+----------+-----+
 * | id | password | age |
 * +----+----------+-----+
 * | 0  | mypwd    | 21  |
 * +----+----------+-----+
 * | 1  | mypwd2   | 8   |
 * +----+----------+-----+
 */
string
login_register(const Json::Value &value_recvd)
{
    try {
        /*
         * Get id and password, even if the id is a number,
         * it should be converted to a string
         */
        string  id  = value_recvd["id"].asString();
        string  pwd = value_recvd["password"].asString();

        if (id.empty() || pwd.empty())
            return "missing id or password";

        /*
         * Query database and get JSON string
         *
         * "8.8.8.8": Database server IP address (/etc/tanserver.conf)
         * row_to_json(): pgSQL JSON function
         */
        string  json_string = pg_query("8.8.8.8",
                                       "select row_to_json(users) from users where id = $1 and password = $2;",
                                       id.c_str(), pwd.c_str());

        /* If json_string is an empty string, it means there is no such user  */
        if (json_string.empty()) {

            /* Complete registration  */
            pg_query("8.8.8.8", "insert into users values($1, $2, $3)",
                     id.c_str(), pwd.c_str(), "16");

            Json::Value  json;

            json["msg"] = "registration successful";

            /* Send {"msg": "registration successful"}  */
            return json_encode(json);
        }

        /* Send JSON string, like: {"id": 0, "password": "mypwd", "age": 21}  */
        return json_string;

    } catch (...) {
        /*
         * Database query failed, the reason will be
         * automatically written to the log file
         *
         * You can choose to throw an exception (disconnect from the client)
         * or send an error JSON string
         *
         * Here we choose to disconnect (it will cause an IOException to be
         * thrown when the Java client calls getJSON())
         */
        throw "error";
    }
}


/*
 * API 5: get_items
 *
 * Convert items table data into JSON string
 * and send it to the client
 *
 * items
 * +----+----------+-------+
 * | id | name     | price |
 * +----+----------+-------+
 * | 0  | book     | 5     |
 * +----+----------+-------+
 * | 1  | keyboard | 200   |
 * +----+----------+-------+
 */
string
get_items(const Json::Value &value_recvd)
{
    try {
        string  json_string = pg_query("8.8.8.8",
                                       "select array_to_json(array_agg(row_to_json(items))) from items;",
                                       NULL);

        json_append_status(json_string, 0, "OK");

        /* Send
         * {
         *      "status":0,
         *      "message":"OK",
         *      "result":[
         *          {
         *              "id":0,
         *              "name":"book",
         *              "price":5
         *          },
         *          {
         *              "id":1,
         *              "name":"keyboard",
         *              "price":200
         *          }
         *      ]
         * }
         */
        return json_string;

        /*
         * Exclude price
         *
         * string  json_string = pg_query("8.8.8.8",
         *                                "select array_to_json(array_agg(json_build_object('id', id, 'name', name))) from items;",
         *                                NULL);
         *
         * json_append_status(json_string, 200, "ok");
         *
         * Send
         * {
         *      "status":200,
         *      "message":"ok",
         *      "result":[
         *          {
         *              "id":0,
         *              "name":"book"
         *          },
         *          {
         *              "id":1,
         *              "name":"keyboard"
         *          }
         *      ]
         * }
         * return json_string;
         */
    } catch (...) {
        throw "error";
    }
}


/* API 6: transaction  */
string
transaction(const Json::Value &value_recvd)
{
    try {
        pg_query("8.8.8.8", "begin;", NULL);

        pg_query("8.8.8.8",
                 "update accounts set num = num - $1 where name = $2;",
                 "10", "Jack");

        pg_query("8.8.8.8",
                 "update accounts set num = num + $1 "
                 "where name = $2;", "10", "Mike");

        pg_query("8.8.8.8", "commit;", NULL);

        /* Send "ok"  */
        return "ok";

    } catch (...) {

        /* Rollback and send "failed"  */
        pg_query("8.8.8.8", "rollback;", NULL);
        return "failed";
    }
}


} /* extern "C" */
