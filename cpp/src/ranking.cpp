#include "ranking.h"

std::vector<node_int> rank_neighbourhood(Graph* graph){
    std::vector<node_int> ranking (graph->get_vertex_nr(), 0);

    for(node_int v = 0; v < graph->get_vertex_nr(); v++){
        ranking[v] = graph->get_adj_matrix()[v].size();
    }

    return ranking;
}

std::vector<node_int> rank_graph(Graph* graph, ranking_algorithm algorithm){
    std::vector<node_int> ranking;

    switch(algorithm){
        case PAGE_RANK_DIRECTED:
            std::cerr << "ranking.cpp: PAGE_RANK_DIRECTED not implemented yet." << std::endl;
            break;
        case PAGE_RANK_UNDIRECTED:
            std::cerr << "ranking.cpp: PAGE_RANK_UNDIRECTED not implemented yet." << std::endl;
            break;
        case NEIGHBOURHOOD:
            ranking = rank_neighbourhood(graph);
            break;
        default:
            std::cerr << "ranking.cpp: No valid ranking algorithm was selected." << std::endl;
            break;
    }

    return ranking;
}