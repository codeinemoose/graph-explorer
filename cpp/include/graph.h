#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <cstdint>

typedef uint16_t node_int;

enum graph_type{
    UNDIRECTED,
    DIRECTED
};

class Graph{
    public:
        Graph(graph_type graph_type, node_int edge_nr, node_int vertex_nr, std::vector<node_int> offsets, std::vector<node_int> targets, std::vector<node_int> degrees, std::vector<node_int> communities):
        graph_type(graph_type), edge_nr(edge_nr), vertex_nr(vertex_nr), offsets(offsets), targets(targets), degrees(degrees), communities(communities){}

        Graph(node_int edge_nr, node_int vertex_nr, std::vector<node_int> offsets, std::vector<node_int> targets, std::vector<node_int> degrees, std::vector<node_int> communities):
        edge_nr(edge_nr), vertex_nr(vertex_nr), offsets(offsets), targets(targets), degrees(degrees), communities(communities){
            this->graph_type = UNDIRECTED;
        }
        
        graph_type get_graph_type();
        node_int get_edge_nr();
        node_int get_vertex_nr();
        std::vector<node_int>& get_offsets();
        std::vector<node_int>& get_targets();
        std::vector<node_int>& get_degrees();
        std::vector<node_int>& get_communities();

        std::vector<node_int> get_neighbors(node_int id);
        std::vector<std::vector<node_int>> get_adj_matrix();

    private:
        graph_type graph_type;
        node_int edge_nr;
        node_int vertex_nr;
        std::vector<node_int> offsets;
        std::vector<node_int> targets;
        std::vector<node_int> degrees;
        std::vector<node_int> communities;
};

#endif