#pragma once
#include <string>
#include <set>
 
struct Student {
    std::string name;
    std::string id;       // 8-digit string
    int residenceLocId;
    std::set<std::string> classes; // class codes
};
