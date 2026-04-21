#include "campusCompass.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include <limits>
#include <iomanip>
#include <cctype>

//trim helper
static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;

    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;

    return s.substr(start, end - start);
}

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

int CampusCompass::toMinutes(const std::string& t) const {
    // expects HH:MM
    if (t.size() != 5 || t[2] != ':') return -1;
    int h = std::stoi(t.substr(0, 2));
    int m = std::stoi(t.substr(3, 2));
    return h * 60 + m;
}

int CampusCompass::mstCost(const std::set<int>& nodeSet) const {
    if (nodeSet.empty()) return 0;
    if (nodeSet.size() == 1) return 0;

    std::set<int> visited;
    auto start = *nodeSet.begin();
    visited.insert(start);

    using State = std::pair<int, std::pair<int, int>>; // weight, {from,to}
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    for (const auto& e : g.neighbours(start)) {
        if (!e.closed && nodeSet.count(e.to)) {
            pq.push({e.weight, {start, e.to}});
        }
    }

    int total = 0;

    while (!pq.empty() && visited.size() < nodeSet.size()) {
        auto [w, edge] = pq.top();
        pq.pop();

        int to = edge.second;
        if (visited.count(to)) continue;

        visited.insert(to);
        total += w;

        for (const auto& e : g.neighbours(to)) {
            if (!e.closed && nodeSet.count(e.to) && !visited.count(e.to)) {
                pq.push({e.weight, {to, e.to}});
            }
        }
    }

    if (visited.size() != nodeSet.size()) return -1;
    return total;
}

//loading the info------------
void CampusCompass::loadEdges(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open edges: " << path << '\n';
        return;
    }

    std::string line;
    int count = 0;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        std::replace(line.begin(), line.end(), '\t', ',');

        std::stringstream ss(line);
        std::vector<std::string> parts;
        std::string part;

        while (std::getline(ss, part, ',')) {
            parts.push_back(trim(part));
        }

        // Expected format:
        // LocationID_1, LocationID_2, Name_1, Name_2, Time
        if (parts.size() < 5) continue;

        int u, v, w;
        try {
            u = std::stoi(parts[0]);
            v = std::stoi(parts[1]);
            w = std::stoi(parts[4]);
        } catch (...) {
            continue; // skips header row too
        }

        g.addNode(u, parts[2]);
        g.addNode(v, parts[3]);
        g.addEdge(u, v, w);
        count++;
    }

    std::cerr << "Loaded edge rows: " << count << '\n';
}

void CampusCompass::loadClasses(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open classes: " << path << '\n';
        return;
    }

    std::string line;
    int count = 0;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        std::replace(line.begin(), line.end(), '\t', ',');

        std::stringstream ss(line);
        std::vector<std::string> parts;
        std::string part;

        while (std::getline(ss, part, ',')) {
            parts.push_back(trim(part));
        }

        if (parts.size() < 4) continue;
        if (!validClassCode(parts[0])) continue;

        int loc;
        try {
            loc = std::stoi(parts[1]);
        } catch (...) {
            continue;
        }

        classLocMap[parts[0]] = loc;
        classTimes[parts[0]] = {parts[2], parts[3]};
        count++;
    }

    std::cerr << "Loaded classes: " << count << '\n';
}

//-----ACTUAL METHODS
void CampusCompass::cmdInsert(std::istringstream& ss) {
    std::string name, id;
    int residenceLocId, n;

    if (!(ss >> std::quoted(name) >> id >> residenceLocId >> n)) {
        std::cout << "unsuccessful";
        return;
    }

    if (!validName(name) || !validId(id) || n < 1 || n > 6) {
        std::cout << "unsuccessful";
        return;
    }

    if (students.count(id)) {
        std::cout << "unsuccessful";
        return;
    }

    std::set<std::string> classes;
    for (int i = 0; i < n; ++i) {
        std::string code;
        if (!(ss >> code)) {
            std::cout << "unsuccessful";
            return;
        }

        if (!validClassCode(code) || !classLocMap.count(code)) {
            std::cout << "unsuccessful";
            return;
        }

        classes.insert(code);
    }

    std::string extra;
    if (ss >> extra) {
        std::cout << "unsuccessful";
        return;
    }

    Student s;
    s.name = name;
    s.id = id;
    s.residenceLocId = residenceLocId;
    s.classes = classes;

    students[id] = s;
    std::cout << "successful";
}

void CampusCompass::cmdRemove(std::istringstream& ss) {
    std::string id;
    if (!(ss >> id) || !students.count(id)) {
        std::cout << "unsuccessful";
        return;
    }

    students.erase(id);
    std::cout << "successful";
}

void CampusCompass::cmdRemoveClass(std::istringstream& ss) {
    std::string code;
    if (!(ss >> code) || !validClassCode(code) || !classLocMap.count(code)) {
        std::cout << "unsuccessful";
        return;
    }

    int count = 0;
    std::vector<std::string> toErase;

    for (auto& [id, student] : students) {
        if (student.classes.count(code)) {
            count++;
            student.classes.erase(code);
            if (student.classes.empty()) {
                toErase.push_back(id);
            }
        }
    }

    for (const auto& id : toErase) {
        students.erase(id);
    }

    if (count == 0) {
        std::cout << "unsuccessful";
    } else {
        std::cout << count;
    }
}

void CampusCompass::cmdToggleEdgesClosure(std::istringstream& ss) {
    int n;
    if (!(ss >> n) || n < 0) {
        std::cout << "unsuccessful";
        return;
    }

    for (int i = 0; i < n; ++i) {
        int u, v;
        if (!(ss >> u >> v)) {
            std::cout << "unsuccessful";
            return;
        }
        g.toggleEdge(u, v);
    }

    std::cout << "successful";
}

void CampusCompass::cmdCheckEdgeStatus(std::istringstream& ss) {
    int u, v;
    if (!(ss >> u >> v)) {
        std::cout << "DNE";
        return;
    }

    std::cout << g.checkEdgeStatus(u, v);
}

void CampusCompass::cmdIsConnected(std::istringstream& ss) {
    int u, v;
    if (!(ss >> u >> v)) {
        std::cout << "unsuccessful";
        return;
    }

    std::cout << (g.isConnected(u, v) ? "successful" : "unsuccessful");
}

void CampusCompass::cmdPrintShortestEdges(std::istringstream& ss) {
    std::string id;
    if (!(ss >> id)) {
        std::cout << "unsuccessful";
        return;
    }

    const Student* sp = nullptr;

    auto it = students.find(id);
    if (it != students.end()) {
        sp = &it->second;
    } else if (students.size() == 1) {
        sp = &students.begin()->second;
    } else {
        std::cout << "unsuccessful";
        return;
    }

    const Student& s = *sp;
    std::vector<std::string> classes(s.classes.begin(), s.classes.end());
    std::sort(classes.begin(), classes.end());

    std::ostringstream out;
    out << "Time For Shortest Edges: " << s.name;
    for (const auto& code : classes) {
        int loc = classLocMap.at(code);
        int dist = g.shortestPath(s.residenceLocId, loc);
        out << "\n" << code << ": " << dist;
    }

    std::cout << out.str();
}

void CampusCompass::cmdPrintStudentZone(std::istringstream& ss) {
    std::string id;
    if (!(ss >> id) || !students.count(id)) {
        std::cout << "unsuccessful";
        return;
    }

    const Student& s = students.at(id);
    std::set<int> nodeSet;
    nodeSet.insert(s.residenceLocId);

    for (const auto& code : s.classes) {
        int classLoc = classLocMap.at(code);
        std::vector<int> path = g.shortestPathNodes(s.residenceLocId, classLoc);
        for (int node : path) {
            nodeSet.insert(node);
        }
    }

    int cost = mstCost(nodeSet);
    std::cout << "Student Zone Cost For " << s.name << ": " << cost;
}

void CampusCompass::cmdVerifySchedule(std::istringstream& ss) {
    std::string id;
    if (!(ss >> id) || !students.count(id)) {
        std::cout << "unsuccessful";
        return;
    }

    const Student& s = students.at(id);
    if (s.classes.size() <= 1) {
        std::cout << "unsuccessful";
        return;
    }

    std::vector<std::string> classes(s.classes.begin(), s.classes.end());
    std::sort(classes.begin(), classes.end(), [&](const std::string& a, const std::string& b) {
        int ta = toMinutes(classTimes.at(a).start);
        int tb = toMinutes(classTimes.at(b).start);
        if (ta != tb) return ta < tb;
        return a < b;
    });

    std::ostringstream out;
    out << "Schedule Check for " << s.name << ":";

    for (size_t i = 0; i + 1 < classes.size(); ++i) {
        const std::string& c1 = classes[i];
        const std::string& c2 = classes[i + 1];

        int end1 = toMinutes(classTimes.at(c1).end);
        int start2 = toMinutes(classTimes.at(c2).start);
        int gap = start2 - end1;

        int loc1 = classLocMap.at(c1);
        int loc2 = classLocMap.at(c2);
        int dist = g.shortestPath(loc1, loc2);

        bool ok = (dist != -1 && gap >= dist);
        out << "\n" << c1 << " - " << c2 << ": " << (ok ? "successful" : "unsuccessful");
    }

    std::cout << out.str();
}

void CampusCompass::cmdReplaceClass(std::istringstream& ss) {
    std::string id, oldCode, newCode;
    if (!(ss >> id >> oldCode >> newCode)) {
        std::cout << "unsuccessful";
        return;
    }

    auto it = students.find(id);
    if (it == students.end()) {
        std::cout << "unsuccessful";
        return;
    }

    if (!it->second.classes.count(oldCode)) {
        std::cout << "unsuccessful";
        return;
    }

    if (!validClassCode(newCode) || !classLocMap.count(newCode)) {
        std::cout << "unsuccessful";
        return;
    }

    if (it->second.classes.count(newCode)) {
        std::cout << "unsuccessful";
        return;
    }

    it->second.classes.erase(oldCode);
    it->second.classes.insert(newCode);
    std::cout << "successful";
}

void CampusCompass::cmdDropClass(std::istringstream& ss) {
    std::string id, code;
    if (!(ss >> id >> code)) {
        std::cout << "unsuccessful";
        return;
    }

    auto it = students.find(id);
    if (it == students.end()) {
        std::cout << "unsuccessful";
        return;
    }

    if (!validClassCode(code) || !classLocMap.count(code)) {
        std::cout << "unsuccessful";
        return;
    }

    if (!it->second.classes.count(code)) {
        std::cout << "unsuccessful";
        return;
    }

    it->second.classes.erase(code);

    if (it->second.classes.empty()) {
        students.erase(it);
    }

    std::cout << "successful";
}

//parsing and processing commands
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
    else                                  std::cout << "unsuccessful";

    std::cout.rdbuf(old);
    return out.str();
}

std::string CampusCompass::processLine(const std::string& line) {
    return processCommand(line);
}

std::string CampusCompass::parseInput(const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;

    int numCommands;
    if (!(in >> numCommands)) return "";
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numCommands; ++i) {
        std::string line;
        std::getline(in, line);

        if (line.empty()) {
            --i;
            continue;
        }

        out << processCommand(line) << "\n";
    }

    return out.str();
}
