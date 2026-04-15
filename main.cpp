#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <regex>
#include <algorithm>
#include <limits>
#include <numeric>
 
#include "Graph.h"
#include "Student.h"
 
// ─── Data ────────────────────────────────────────────────────────────────────
Graph g;
std::unordered_map<std::string, Student> students;    // id -> Student
std::unordered_map<std::string, int>     classLocMap; // classcode -> locationId
// Extra-credit: class times
struct ClassTime { std::string start, end; };
std::unordered_map<std::string, ClassTime> classTimes;
 
// ─── Validation helpers ───────────────────────────────────────────────────────
bool validId(const std::string& id) {
    if (id.size() != 8) return false;
    return std::all_of(id.begin(), id.end(), ::isdigit);
}
 
bool validName(const std::string& name) {
    if (name.empty()) return false;
    for (char c : name)
        if (!isalpha(c) && c != ' ') return false;
    return true;
}
 
bool validClassCode(const std::string& code) {
    if (code.size() != 7) return false;
    for (int i = 0; i < 3; ++i) if (!isupper(code[i])) return false;
    for (int i = 3; i < 7; ++i) if (!isdigit(code[i])) return false;
    return true;
}
 
// ─── CSV Loaders ─────────────────────────────────────────────────────────────
void loadEdges(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) { std::cerr << "Cannot open " << path << "\n"; return; }
    std::string line;
    std::getline(f, line); // header
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string tok;
        std::vector<std::string> tokens;
        while (std::getline(ss, tok, ',')) tokens.push_back(tok);
        if (tokens.size() < 5) continue;
        int id1  = std::stoi(tokens[0]);
        int id2  = std::stoi(tokens[1]);
        std::string name1 = tokens[2];
        std::string name2 = tokens[3];
        int time = std::stoi(tokens[4]);
        g.addNode(id1, name1);
        g.addNode(id2, name2);
        g.addEdge(id1, id2, time);
    }
}
 
void loadClasses(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) { std::cerr << "Cannot open " << path << "\n"; return; }
    std::string line;
    std::getline(f, line); // header
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string tok;
        std::vector<std::string> tokens;
        while (std::getline(ss, tok, ',')) tokens.push_back(tok);
        if (tokens.size() < 2) continue;
        std::string code = tokens[0];
        int locId = std::stoi(tokens[1]);
        classLocMap[code] = locId;
        if (tokens.size() >= 4)
            classTimes[code] = {tokens[2], tokens[3]};
    }
}
 
// ─── Command handlers ─────────────────────────────────────────────────────────
 
// insert "Name" ID RESIDENCE_LOC N CLASS1 CLASS2 ...
void cmdInsert(std::istringstream& ss) {
    std::string nameToken, idStr, residStr, nStr;
 
    // parse quoted name
    std::string nameRaw;
    ss >> nameRaw;
    if (nameRaw.front() != '"') { std::cout << "unsuccessful\n"; return; }
    if (nameRaw.back() == '"') {
        nameRaw = nameRaw.substr(1, nameRaw.size() - 2);
    } else {
        // name has spaces
        nameRaw = nameRaw.substr(1);
        std::string part;
        while (ss >> part) {
            if (part.back() == '"') {
                nameRaw += " " + part.substr(0, part.size() - 1);
                break;
            }
            nameRaw += " " + part;
        }
    }
 
    if (!validName(nameRaw)) { std::cout << "unsuccessful\n"; return; }
 
    ss >> idStr;
    if (!validId(idStr)) { std::cout << "unsuccessful\n"; return; }
    if (students.count(idStr)) { std::cout << "unsuccessful\n"; return; }
 
    int residLoc;
    ss >> residLoc;
    if (!g.hasNode(residLoc)) { std::cout << "unsuccessful\n"; return; }
 
    int n;
    ss >> n;
 
    std::set<std::string> classes;
    for (int i = 0; i < n; ++i) {
        std::string code;
        if (!(ss >> code)) { std::cout << "unsuccessful\n"; return; }
        if (!validClassCode(code)) { std::cout << "unsuccessful\n"; return; }
        if (!classLocMap.count(code)) { std::cout << "unsuccessful\n"; return; }
        classes.insert(code);
    }
 
    if (n < 1 || n > 6 || (int)classes.size() != n) {
        std::cout << "unsuccessful\n"; return;
    }
 
    students[idStr] = {nameRaw, idStr, residLoc, classes};
    std::cout << "successful\n";
}
 
void cmdRemove(std::istringstream& ss) {
    std::string idStr; ss >> idStr;
    if (!students.count(idStr)) { std::cout << "unsuccessful\n"; return; }
    students.erase(idStr);
    std::cout << "successful\n";
}
 
void cmdDropClass(std::istringstream& ss) {
    std::string idStr, code; ss >> idStr >> code;
    if (!students.count(idStr)) { std::cout << "unsuccessful\n"; return; }
    if (!validClassCode(code)) { std::cout << "unsuccessful\n"; return; }
    auto& s = students[idStr];
    if (!s.classes.count(code)) { std::cout << "unsuccessful\n"; return; }
    s.classes.erase(code);
    if (s.classes.empty()) students.erase(idStr);
    std::cout << "successful\n";
}
 
void cmdReplaceClass(std::istringstream& ss) {
    std::string idStr, c1, c2; ss >> idStr >> c1 >> c2;
    if (!students.count(idStr)) { std::cout << "unsuccessful\n"; return; }
    if (!validClassCode(c1) || !validClassCode(c2)) { std::cout << "unsuccessful\n"; return; }
    if (!classLocMap.count(c2)) { std::cout << "unsuccessful\n"; return; }
    auto& s = students[idStr];
    if (!s.classes.count(c1)) { std::cout << "unsuccessful\n"; return; }
    if (s.classes.count(c2)) { std::cout << "unsuccessful\n"; return; }
    s.classes.erase(c1);
    s.classes.insert(c2);
    std::cout << "successful\n";
}
 
void cmdRemoveClass(std::istringstream& ss) {
    std::string code; ss >> code;
    if (!validClassCode(code)) { std::cout << "unsuccessful\n"; return; }
    if (!classLocMap.count(code)) { std::cout << "unsuccessful\n"; return; }
 
    int count = 0;
    std::vector<std::string> toErase;
    for (auto& [id, s] : students) {
        if (s.classes.count(code)) {
            s.classes.erase(code);
            ++count;
            if (s.classes.empty()) toErase.push_back(id);
        }
    }
    if (count == 0) { std::cout << "unsuccessful\n"; return; }
    for (auto& id : toErase) students.erase(id);
    std::cout << count << "\n";
}
 
void cmdToggleEdgesClosure(std::istringstream& ss) {
    int n; ss >> n;
    for (int i = 0; i < n; ++i) {
        int u, v; ss >> u >> v;
        g.toggleEdge(u, v);
    }
    std::cout << "successful\n";
}
 
void cmdCheckEdgeStatus(std::istringstream& ss) {
    int u, v; ss >> u >> v;
    std::cout << g.checkEdgeStatus(u, v) << "\n";
}
 
void cmdIsConnected(std::istringstream& ss) {
    int u, v; ss >> u >> v;
    std::cout << (g.isConnected(u, v) ? "successful" : "unsuccessful") << "\n";
}
 
void cmdPrintShortestEdges(std::istringstream& ss) {
    std::string idStr; ss >> idStr;
    if (!students.count(idStr)) { std::cout << "unsuccessful\n"; return; }
    const auto& s = students[idStr];
    std::cout << "Time For Shortest Edges: " << s.name << "\n";
    for (const auto& code : s.classes) {  // set is already sorted lexicographically
        int loc = classLocMap.at(code);
        int dist = g.shortestPath(s.residenceLocId, loc);
        std::cout << code << ": " << dist << "\n";
    }
}
 
// ─── Kruskal MST helpers ───────────────────────────────────────────────────────
struct DSU {
    std::unordered_map<int,int> parent, rank_;
    void make(int x) { parent[x] = x; rank_[x] = 0; }
    int find(int x) {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (rank_[a] < rank_[b]) std::swap(a, b);
        parent[b] = a;
        if (rank_[a] == rank_[b]) rank_[a]++;
        return true;
    }
};
 
int mstCost(const std::set<int>& nodeSet) {
    // collect all edges between nodes in nodeSet
    struct WEdge { int u, v, w; };
    std::vector<WEdge> edges;
    for (int u : nodeSet) {
        for (const auto& e : g.neighbours(u)) {
            if (e.closed) continue;
            if (nodeSet.count(e.to) && u < e.to)
                edges.push_back({u, e.to, e.weight});
        }
    }
    std::sort(edges.begin(), edges.end(), [](const WEdge& a, const WEdge& b){ return a.w < b.w; });
 
    DSU dsu;
    for (int n : nodeSet) dsu.make(n);
 
    int total = 0;
    for (auto& e : edges) {
        if (dsu.unite(e.u, e.v)) total += e.w;
    }
    return total;
}
 
void cmdPrintStudentZone(std::istringstream& ss) {
    std::string idStr; ss >> idStr;
    if (!students.count(idStr)) { std::cout << "unsuccessful\n"; return; }
    const auto& s = students[idStr];
 
    // Step 1: get shortest path node lists for each class
    std::set<int> allNodes;
    allNodes.insert(s.residenceLocId);
 
    for (const auto& code : s.classes) {
        int loc = classLocMap.at(code);
        auto path = g.shortestPathNodes(s.residenceLocId, loc);
        for (int n : path) allNodes.insert(n);
    }
 
    // Step 2 & 3: subgraph = edges between allNodes; compute MST
    int cost = mstCost(allNodes);
    std::cout << "Student Zone Cost For " << s.name << ": " << cost << "\n";
}
 
// ─── Extra Credit: verifySchedule ─────────────────────────────────────────────
// Parse "HH:MM" to total minutes
int toMinutes(const std::string& t) {
    int h = std::stoi(t.substr(0, 2));
    int m = std::stoi(t.substr(3, 2));
    return h * 60 + m;
}
 
void cmdVerifySchedule(std::istringstream& ss) {
    std::string idStr; ss >> idStr;
    if (!students.count(idStr)) { std::cout << "unsuccessful\n"; return; }
    const auto& s = students[idStr];
 
    if (s.classes.size() == 1) { std::cout << "unsuccessful\n"; return; }
 
    // Sort classes by start time
    std::vector<std::string> sorted(s.classes.begin(), s.classes.end());
    std::sort(sorted.begin(), sorted.end(), [](const std::string& a, const std::string& b){
        return toMinutes(classTimes.at(a).start) < toMinutes(classTimes.at(b).start);
    });
 
    std::cout << "Schedule Check for " << s.name << ":\n";
    for (int i = 0; i + 1 < (int)sorted.size(); ++i) {
        const auto& c1 = sorted[i];
        const auto& c2 = sorted[i+1];
        int gap = toMinutes(classTimes.at(c2).start) - toMinutes(classTimes.at(c1).end);
        int travel = g.shortestPath(classLocMap.at(c1), classLocMap.at(c2));
        bool ok = travel >= 0 && gap >= travel;
        std::cout << c1 << " - " << c2 << ": " << (ok ? "successful" : "unsuccessful") << "\n";
    }
}
 
// ─── Main ─────────────────────────────────────────────────────────────────────
int main() {
    loadEdges("data/edges.csv");
    loadClasses("data/classes.csv");
 
    int numCommands;
    std::cin >> numCommands;
    std::cin.ignore();
 
    for (int i = 0; i < numCommands; ++i) {
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) { --i; continue; }
 
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;
 
        if      (cmd == "insert")               cmdInsert(ss);
        else if (cmd == "remove")               cmdRemove(ss);
        else if (cmd == "dropClass")            cmdDropClass(ss);
        else if (cmd == "replaceClass")         cmdReplaceClass(ss);
        else if (cmd == "removeClass")          cmdRemoveClass(ss);
        else if (cmd == "toggleEdgesClosure")   cmdToggleEdgesClosure(ss);
        else if (cmd == "checkEdgeStatus")      cmdCheckEdgeStatus(ss);
        else if (cmd == "isConnected")          cmdIsConnected(ss);
        else if (cmd == "printShortestEdges")   cmdPrintShortestEdges(ss);
        else if (cmd == "printStudentZone")     cmdPrintStudentZone(ss);
        else if (cmd == "verifySchedule")       cmdVerifySchedule(ss);
        else                                    std::cout << "unsuccessful\n";
    }
 
    return 0;
}
