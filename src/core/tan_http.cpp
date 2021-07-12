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
    std::cmatch  matches;
    std::string  rgx_str(": (.+)\r\n");

    rgx_str = std::string(field) + rgx_str;

    std::regex  rgx(rgx_str);

    if (std::regex_search(header, matches, rgx))
        return matches[1].str();  /* Index 1 to get first group.  */

    throw "Field not found.";
}
