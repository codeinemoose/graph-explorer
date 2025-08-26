#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <cstdint>

typedef uint16_t node_int;

class Graph{
    public:
        Graph(node_int edge_nr, node_int vertex_nr, std::vector<node_int> offsets, std::vector<node_int> targets, std::vector<node_int> degrees, std::vector<node_int> communities):
        edge_nr(edge_nr), vertex_nr(vertex_nr), offsets(offsets), targets(targets), degrees(degrees), communities(communities){}
        
        node_int get_edge_nr();
        node_int get_vertex_nr();
        std::vector<node_int>& get_offsets();
        std::vector<node_int>& get_targets();
        std::vector<node_int>& get_degrees();
        std::vector<node_int>& get_communities();

        std::vector<node_int> get_neighbors(node_int id);
        std::vector<std::vector<node_int>> get_adj_matrix();

    private:
        node_int edge_nr;
        node_int vertex_nr;
        std::vector<node_int> offsets;
        std::vector<node_int> targets;
        std::vector<node_int> degrees;
        std::vector<node_int> communities;
};

enum graph_type{
    undirected_graph,
    directed_graph
};

#endif