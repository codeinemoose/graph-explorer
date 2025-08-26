/**
 * @brief This file aims to implement the label propogation algorithm in order to assign communities
 * to a set of nodes in a Graph.
 */

#include "labelprop.h"
#include "graph.h"
#include "config.h"
#include <vector>
#include <algorithm>
#include <iostream>


void propagate(Graph* graph, node_int node_index) {
    auto& offsets = graph->get_offsets();
    auto& targets = graph->get_targets();
    auto& communities = graph->get_communities();

    node_int begin_index = offsets[node_index];
    node_int end_index   = offsets[node_index + 1];

    if (begin_index == end_index) return; // isolated node, nothing to do

    // copy neighbors to temp vector for sorting
    std::vector<node_int> neighbors(targets.begin() + begin_index, targets.begin() + end_index);
    std::sort(neighbors.begin(), neighbors.end());

    node_int max_count = 1;
    node_int var_count = 1;
    std::vector<node_int> final_elements = { communities[neighbors[0]] };

    for (size_t n = 1; n < neighbors.size(); n++) {
        if (communities[neighbors[n]] == communities[neighbors[n-1]]) {
            var_count++;
        } else {
            var_count = 1;
        }

        if (var_count > max_count) {
            max_count = var_count;
            final_elements.clear();
            final_elements.push_back(communities[neighbors[n]]);
        } else if (var_count == max_count) {
            final_elements.push_back(communities[neighbors[n]]);
        }
    }

    if (final_elements.size() == 1) {
        communities[node_index] = final_elements.back();
    } else if (!final_elements.empty()) {
        communities[node_index] = final_elements[std::rand() % final_elements.size()];
    }
}


int label_prop(Graph* graph){
    node_int vertex_nr = graph->get_vertex_nr();
    //node_int edge_nr = graph->get_edge_nr();

    for(node_int r = 0; r < config::PROP_STEPS_PER_ITER; r++){
        for(node_int i = 0; i < vertex_nr; i++){
            propagate(graph, i);
        }
    }

    return 1;
}