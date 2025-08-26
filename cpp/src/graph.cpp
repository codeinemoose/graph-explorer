#include <vector>
#include "graph.h"



node_int Graph::get_vertex_nr(){
    return this->vertex_nr;
}

node_int Graph::get_edge_nr(){
    return this->edge_nr;
}

std::vector<node_int>& Graph::get_degrees(){
    return degrees;
}

std::vector<node_int>& Graph::get_offsets(){
    return offsets;
}

std::vector<node_int>& Graph::get_targets(){
    return targets;
}

std::vector<node_int>& Graph::get_communities(){
    return communities;
}

std::vector<node_int> Graph::get_neighbors(node_int id){
    auto& offsets = this->get_offsets();
    auto& targets = this->get_targets();
    //auto& communities = this->get_communities();

    node_int begin_index = offsets[id];
    node_int end_index   = offsets[id + 1];

    if (begin_index == end_index) return {}; // isolated node, nothing to do

    // copy neighbors to temp vector for sorting
    std::vector<node_int> neighbors(targets.begin() + begin_index, targets.begin() + end_index);

    return neighbors;
}

std::vector<std::vector<node_int>> Graph::get_adj_matrix(){
    std::vector<std::vector<node_int>> adj_matrix;

    for(node_int i = 0; i < this->get_vertex_nr(); i++){
        adj_matrix.push_back(this->get_neighbors(i));
    }

    return adj_matrix;
}