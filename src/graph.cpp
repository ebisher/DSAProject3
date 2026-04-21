#include "graph.h"
#include <queue>
#include <algorithm>
#include <stdexcept>
 
void Graph::addNode(int id, const std::string& name) {
    nodeNames_[id] = name;
    if (adj_.find(id) == adj_.end())
        adj_[id] = {};
}
 
void Graph::addEdge(int u, int v, int weight) {
    // ensure nodes exist
    if (adj_.find(u) == adj_.end()) adj_[u] = {};
    if (adj_.find(v) == adj_.end()) adj_[v] = {};
 
    //make sure no dupes
    for (auto& e : adj_[u]) if (e.to == v) return;
    adj_[u].push_back({v, weight, false});
    adj_[v].push_back({u, weight, false});
}
 
Edge* Graph::findEdge(int u, int v) {
    auto it = adj_.find(u);
    if (it == adj_.end()) return nullptr;
    for (auto& e : it->second)
        if (e.to == v) return &e;
    return nullptr;
}
 
const Edge* Graph::findEdge(int u, int v) const {
    auto it = adj_.find(u);
    if (it == adj_.end()) return nullptr;
    for (const auto& e : it->second)
        if (e.to == v) return &e;
    return nullptr;
}
 
bool Graph::toggleEdge(int u, int v) {
    Edge* eu = findEdge(u, v);
    Edge* ev = findEdge(v, u);
    if (!eu || !ev) return false;
    eu->closed = !eu->closed;
    ev->closed = !ev->closed;
    return true;
}
 
std::string Graph::checkEdgeStatus(int u, int v) const {
    const Edge* e = findEdge(u, v);
    if (!e) return "DNE";
    return e->closed ? "closed" : "open";
}
 
bool Graph::isConnected(int u, int v) const {
    if (!hasNode(u) || !hasNode(v)) return false;
    if (u == v) return true;
 
    std::unordered_set<int> visited;
    std::queue<int> q;
    q.push(u);
    visited.insert(u);
 //traversal ohh yeah
    while (!q.empty()) {
        int cur = q.front(); q.pop();
        auto it = adj_.find(cur);
        if (it == adj_.end()) continue;
        for (const auto& e : it->second) {
            if (e.closed) continue;
            if (e.to == v) return true;
            if (!visited.count(e.to)) {
                visited.insert(e.to);
                q.push(e.to);
            }
        }
    }
    return false;
}
 
int Graph::shortestPath(int src, int dst) const {
    if (!hasNode(src) || !hasNode(dst)) return -1;
    if (src == dst) return 0;
 
    const int INF = std::numeric_limits<int>::max();
    std::unordered_map<int, int> dist;
    for (auto& [id, _] : adj_) dist[id] = INF;
    dist[src] = 0;
 
    // min-heap: (dist, node)
    std::priority_queue<std::pair<int,int>,
                        std::vector<std::pair<int,int>>,
                        std::greater<>> pq;
    pq.push({0, src});
 
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        if (u == dst) return d;
        auto it = adj_.find(u);
        if (it == adj_.end()) continue;
        for (const auto& e : it->second) {
            if (e.closed) continue;
            int nd = d + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                pq.push({nd, e.to});
            }
        }
    }
    return dist.count(dst) && dist.at(dst) != INF ? dist.at(dst) : -1;
}
 
std::vector<int> Graph::shortestPathNodes(int src, int dst) const {
    if (!hasNode(src) || !hasNode(dst)) return {};
    if (src == dst) return {src};
    const int INF = std::numeric_limits<int>::max();
    std::unordered_map<int, int> dist;
    std::unordered_map<int, int> prev;
    for (auto& [id, _] : adj_) { dist[id] = INF; prev[id] = -1; }
    dist[src] = 0;
    std::priority_queue<std::pair<int,int>,
                        std::vector<std::pair<int,int>>,
                        std::greater<>> pq;
    pq.push({0, src});
     while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        auto it = adj_.find(u);
        if (it == adj_.end()) continue;
        for (const auto& e : it->second) {
            if (e.closed) continue;
            int nd = d + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                prev[e.to] = u;
                pq.push({nd, e.to});
            }
        }
    }
    if (!dist.count(dst) || dist.at(dst) == INF) return {};
    std::vector<int> path;
    for (int cur = dst; cur != -1; cur = prev[cur])
        path.push_back(cur);
    std::reverse(path.begin(), path.end());
    return path;
}
 
bool Graph::hasNode(int id) const {
    return adj_.find(id) != adj_.end();
}
 
std::string Graph::nodeName(int id) const {
    auto it = nodeNames_.find(id);
    return it != nodeNames_.end() ? it->second : std::to_string(id);
}
 
const std::vector<Edge>& Graph::neighbours(int id) const {
    static const std::vector<Edge> empty;
    auto it = adj_.find(id);
    return it != adj_.end() ? it->second : empty;
}
 
std::vector<int> Graph::allNodes() const {
    std::vector<int> nodes;
    nodes.reserve(adj_.size());
    for (auto& [id, _] : adj_) nodes.push_back(id);
    return nodes;
}
