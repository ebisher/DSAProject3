#include "campusCompass.h"
#include <sstream>
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

#include <sstream>
#include <iostream>

std::string CampusCompass::processCommand(const std::string& line) {
    std::ostringstream out;
    std::streambuf* old = std::cout.rdbuf(out.rdbuf());

    std::istringstream ss(line);
    std::string cmd;
    ss >> cmd;

    if      (cmd == "insert")             cmdInsert(ss);
    else if (cmd == "remove")             cmdRemove(ss);
    else if (cmd == "dropClass")          cmdDropClass(ss);
    else if (cmd == "replaceClass")       cmdReplaceClass(ss);
    else if (cmd == "removeClass")        cmdRemoveClass(ss);
    else if (cmd == "toggleEdgesClosure") cmdToggleEdgesClosure(ss);
    else if (cmd == "checkEdgeStatus")    cmdCheckEdgeStatus(ss);
    else if (cmd == "isConnected")        cmdIsConnected(ss);
    else if (cmd == "printShortestEdges") cmdPrintShortestEdges(ss);
    else if (cmd == "printStudentZone")   cmdPrintStudentZone(ss);
    else if (cmd == "verifySchedule")     cmdVerifySchedule(ss);
    else                                  std::cout << "unsuccessful\n";

    std::cout.rdbuf(old);
    return out.str();
}

std::string CampusCompass::processLine(const std::string& line) {
    return processCommand(line);
}

std::string CampusCompass::parseInput(const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;

    std::streambuf* old = std::cout.rdbuf(out.rdbuf());

    int numCommands;
    in >> numCommands;
    in.ignore();

    for (int i = 0; i < numCommands; ++i) {
        std::string line;
        std::getline(in, line);

        if (line.empty()) {
            --i;
            continue;
        }

        processCommand(line);
    }

    std::cout.rdbuf(old);
    return out.str();
}