#pragma once
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>
#include <string>
#include <limits>
#include <set>
 
struct Edge {
    int to;
    int weight;
    bool closed;
};
 
class Graph {
public:
    //Adding a location node
    void addNode(int id, const std::string& name);
 
    //Add an undir edge
    void addEdge(int u, int v, int weight);
 
    //Edge closure
    bool toggleEdge(int u, int v);
 
    //Check edge status
    std::string checkEdgeStatus(int u, int v) const;
 
    //Check if two nodes are connected using only open edges
    bool isConnected(int u, int v) const;
 
    //Dijkstra
    int shortestPath(int src, int dst) const;
 
    // Dijkstra returning the full path but e if unreachable
    std::vector<int> shortestPathNodes(int src, int dst) const;
 
    //check existence of node
    bool hasNode(int id) const;
 
    //getter for node name
    std::string nodeName(int id) const;
 
    //All neighbour edges for node
    const std::vector<Edge>& neighbours(int id) const;
 
    //All node ids
    std::vector<int> allNodes() const;
 
private:
    std::unordered_map<int, std::string> nodeNames_;
    std::unordered_map<int, std::vector<Edge>> adj_;
 
    //Find the edge iterator
    Edge* findEdge(int u, int v);
    const Edge* findEdge(int u, int v) const;
};
