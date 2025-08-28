#ifndef RANKING_H
#define RANKING_H

#include <vector>
#include <iostream>
#include "config.h"
#include "graph.h"

enum ranking_algorithm{
    /**
     * @brief Implements PageRank for directed graphs.
     */
    PAGE_RANK_DIRECTED,
    /**
     * @brief Implements PageRank for undirected graphs.
     */
    PAGE_RANK_UNDIRECTED,
    /**
     * @brief Gives each node a rank based on how many neighbours that node has (e.g. it's neighbourhood).
     */
    NEIGHBOURHOOD
};

std::vector<node_int> rank_graph(Graph* graph, ranking_algorithm algorithm);

#endif