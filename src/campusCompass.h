#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <sstream>

#include "graph.h"
#include "student.h"

struct ClassTime {
    std::string start;
    std::string end;
};

class CampusCompass {
private:
    Graph g;
    std::unordered_map<std::string, Student> students;
    std::unordered_map<std::string, int> classLocMap;
    std::unordered_map<std::string, ClassTime> classTimes;

    bool validId(const std::string& id) const;
    bool validName(const std::string& name) const;
    bool validClassCode(const std::string& code) const;
    int toMinutes(const std::string& t) const;
    int mstCost(const std::set<int>& nodeSet) const;

public:
    CampusCompass() = default;

    void loadEdges(const std::string& path);
    void loadClasses(const std::string& path);

    void cmdInsert(std::istringstream& ss);
    void cmdRemove(std::istringstream& ss);
    void cmdDropClass(std::istringstream& ss);
    void cmdReplaceClass(std::istringstream& ss);
    void cmdRemoveClass(std::istringstream& ss);
    void cmdToggleEdgesClosure(std::istringstream& ss);
    void cmdCheckEdgeStatus(std::istringstream& ss);
    void cmdIsConnected(std::istringstream& ss);
    void cmdPrintShortestEdges(std::istringstream& ss);
    void cmdPrintStudentZone(std::istringstream& ss);
    void cmdVerifySchedule(std::istringstream& ss);

    std::string processLine(const std::string& line);
    std::string processCommand(const std::string& line);
    std::string parseInput(const std::string& input);  //needed for last test

    const std::unordered_map<std::string, Student>& getStudents() const { return students; }
    const Graph& getGraph() const { return g; }
};