#include <string>
#include <map>
#include <set>

#ifndef _PARSE_UTIL_
#define _PARSE_UTIL_

std::set<std::string> remove_reserved_variables(
    std::set<std::string> variables,
    std::set<std::string> additional_reserved);

std::set<std::string> gather_variables(const std::string &line);

std::string redefine_singleline_macro(const std::string &contents,
                                      const std::map<std::string, std::string>
                                      &macro_new_defs);

std::string get_file_contents(const std::string &fname);

#endif