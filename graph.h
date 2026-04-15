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
    bool closed; // soft-delete
};
 
class Graph {
public:
    // Add a location node with its name
    void addNode(int id, const std::string& name);
 
    // Add an undirected edge between two nodes
    void addEdge(int u, int v, int weight);
 
    // Toggle edge closure (soft delete). Returns false if edge DNE.
    bool toggleEdge(int u, int v);
 
    // Check edge status: "open", "closed", "DNE"
    std::string checkEdgeStatus(int u, int v) const;
 
    // Check if two nodes are connected using only open edges (BFS/DFS)
    bool isConnected(int u, int v) const;
 
    // Dijkstra: shortest path using only open edges. Returns -1 if unreachable.
    int shortestPath(int src, int dst) const;
 
    // Dijkstra returning the full path (node list). Empty if unreachable.
    std::vector<int> shortestPathNodes(int src, int dst) const;
 
    // Node exists?
    bool hasNode(int id) const;
 
    // Get node name
    std::string nodeName(int id) const;
 
    // Get all neighbour edges for a node
    const std::vector<Edge>& neighbours(int id) const;
 
    // All node ids
    std::vector<int> allNodes() const;
 
private:
    std::unordered_map<int, std::string> nodeNames_;
    std::unordered_map<int, std::vector<Edge>> adj_;
 
    // Helper: find edge iterator
    Edge* findEdge(int u, int v);
    const Edge* findEdge(int u, int v) const;
};
