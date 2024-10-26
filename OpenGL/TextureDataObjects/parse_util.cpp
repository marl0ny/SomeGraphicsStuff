#include "parse_util.hpp"

#include <regex>
#include <iostream>
#include <fstream>


std::set<std::string> 
remove_reserved_variables(std::set<std::string> variables,
                          std::set<std::string> additional_reserved) {
    std::set<std::string> reserved = {
        "sin", "cos", "tan",
        "asin", "acos", "atan",
        "sinh", "cosh", "tanh",
        "asinh", "acosh", "atanh",
        "pow", "log", "exp",
        "sqrt", "inversesqrt",
        "abs", "ceil", "max", "min",
        "mod", "modf", "pi",
        "circle"
    };
    for (auto &v: additional_reserved) {
        reserved.insert(v);
    }
    std::set<std::string> variables2 = {};
    for (auto &v: variables) {
        // https://en.cppreference.com/w/cpp/container/set/find
        auto s = reserved.find(v);
        if (s == reserved.end())
            variables2.insert(v);
    }
    return variables2;
}


std::set<std::string> gather_variables(const std::string &line) {
    auto line2 = line + " ";
    auto variables = std::set<std::string> {};
    auto reg_var = std::regex("[a-zA-Z_][a-zA-Z_0-9]*[^a-zA-Z_0-9]");
    // https://en.cppreference.com/w/cpp/regex/regex_iterator
    auto start = std::sregex_iterator(line2.begin(), line2.end(), reg_var);
    auto end = std::sregex_iterator();
    for (std::sregex_iterator i = start; i != end; i++) {
        auto s = std::string(i->str());
        s.pop_back();
        variables.insert(s);
    }
    return variables;
}

std::string redefine_singleline_macro(const std::string &contents,
                                      const std::map<std::string, std::string>
                                      &macro_new_defs) {
    /* TODO: There's a bug involving the end line of the input file.
    */
    // auto reg = std::regex("define " + macro_name + ".*");
    // auto rep = "#define " + macro_name + " " + new_macro_definition;
    std::string new_contents = "";
    std::string line = "";
    for (auto &c: contents) {
        if (c == '\n') {
            bool end_loop = false;
            for (auto it = macro_new_defs.begin(); 
                 it != macro_new_defs.end() && !end_loop; it++) {
                std::string macro_name = it->first;
                std::string new_macro_definition = it->second;
                // std::cout << new_macro_definition << std::endl;
                auto reg = std::regex("#define[ ]+" + macro_name + ".*");
                auto repl = "#define " + macro_name 
                    + " " + new_macro_definition;
                std::smatch match;
                if (std::regex_search(line, match, reg)) {
                    // std::cout << "This is reached" << std::endl;
                    new_contents += repl + "\n";
                    end_loop = !end_loop;
                }
            }
            if (!end_loop)
                new_contents += line + "\n";
            line = "";
        } else {
            line += c;
        }
    }
    return new_contents;
}

std::string get_file_contents(const std::string &fname) {
    auto s = std::string {};
    std::filebuf fb;
    if (!fb.open(fname, std::ios::in)) {
        std::cout << "Opening " << fname << " failed." << std::endl;
        fb.close();
    }
    for (; fb.sgetc() > 0; fb.snextc())
        s.push_back(fb.sgetc());
    s.push_back('\0');
    fb.close();
    return s;
}
