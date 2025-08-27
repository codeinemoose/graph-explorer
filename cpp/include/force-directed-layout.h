#ifndef FORCE_DIRECTED_LAYOUT_H
#define FORCE_DIRECTED_LAYOUT_H

#include <string>
#include "graph.h"
#include "config.h"

class FDL{
    public:
        FDL(std::vector<std::pair<double,double>> pos, std::vector<std::pair<double,double>> dis, std::vector<std::vector<node_int>> adj_matrix,
            Graph* graph, const int width, const int height, const int area, const int max_iter, const double k, double temp) 
            : pos(pos), dis(dis), adj_matrix(adj_matrix), graph(graph), width(width), height(height), area(area), max_iter(max_iter),
            k(k), temp(temp){}

        std::vector<std::pair<double,double>> pos;
        std::vector<std::pair<double,double>> dis;
        std::vector<std::vector<node_int>> adj_matrix;
        Graph* graph;
        const int width;
        const int height;
        const int area;
        const int max_iter;
        const double k;
        double temp;
};

void fdl_run(std::string file_name, Graph* graph);

#endif