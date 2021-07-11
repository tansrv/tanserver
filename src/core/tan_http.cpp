/*
 * Copyright (C) tanserver.org
 * Copyright (C) Daniele Affinita
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_http.h"

#include <regex>


std::string
tan_http_header_get_value(const char *header,
                          const char *field)
{
    std::string rgxStr (": (.+)\r\n");
    rgxStr = std::string(field) + rgxStr;

    std::cmatch matches;
    std::regex rgx (rgxStr);

    if(std::regex_search(header, matches, rgx))
        return matches[1].first; // index 1 to get first group

    throw "error";
}
