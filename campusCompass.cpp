#include "campus_compass.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include <limits>

//validation helprs
bool CampusCompass::validId(const std::string& id) const {
    if (id.size() != 8) return false;
    return std::all_of(id.begin(), id.end(), ::isdigit);
}

bool CampusCompass::validName(const std::string& name) const {
    if (name.empty()) return false;
    for (char c : name) {
        if (!isalpha(c) && c != ' ') return false;
    }
    return true;
}

bool CampusCompass::validClassCode(const std::string& code) const {
    if (code.size() != 7) return false;
    for (int i = 0; i < 3; ++i) if (!isupper(code[i])) return false;
    for (int i = 3; i < 7; ++i) if (!isdigit(code[i])) return false;
    return true;
}
