#include "tan_core.h"


char *
tan_http_header_get_value(const char *header,
                          const char *key)
{
    std::string rgxStr (": (.+)\r\n");
    rgxStr = std::string(key) + rgxStr;

    std::cmatch matches;
    std::regex rgx (rgxStr);
    
    if(std::regex_search(header, matches, rgx))
        return (char*) matches[1].first; // index 1 to get first group

    return NULL;
}
