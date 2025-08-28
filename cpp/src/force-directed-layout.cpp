/**
 * @brief Implements Fruchtermann and Reingold. (see: https://cs.brown.edu/people/rtamassi/gdhandbook/chapters/force-directed.pdf)
 */
#include "graph.h"
#include "force-directed-layout.h"
#include "main.h"
#include "config.h"
#include <math.h>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>

/**
 * @brief Repulsive force
 */
double f_att(double x, double k){
    return (x*x)/k;
}

/**
 * @brief Attractive force
 */
double f_rep(double x, double k){
    return (k*k)/x;
}

/**
 * @brief Calculates the Euclidean distance between two points.
 */
/*
double length(double a, double b){
    return std::abs(a - b);
}
*/

double cool(double temp, int iteration){
    return fdl::FDL_START_TEMP * (1.0 - (double)iteration / (double)fdl::FDL_MAX_ITER);
}

static inline double length(double dx, double dy) {
    return std::hypot(dx, dy);
}

void fdl_iteration(FDL *fdl, Graph* graph, int iteration){
    const double EPS = 1e-9;
    const node_int n = graph->get_vertex_nr();

    // reset displacements
    for(node_int v = 0; v < n; ++v) {
        fdl->dis[v] = {0.0, 0.0};
    }

    // repulsive forces
    for(node_int v = 0; v < n; ++v){
        for(node_int u = 0; u < n; ++u){
            if(u == v) continue;

            double dx = fdl->pos[v].first  - fdl->pos[u].first;
            double dy = fdl->pos[v].second - fdl->pos[u].second;
            double d  = length(dx, dy);

            if(d < EPS) {
                dx = ((double)std::rand() / RAND_MAX - 0.5) * 1e-3;
                dy = ((double)std::rand() / RAND_MAX - 0.5) * 1e-3;
                d = length(dx, dy);
                if (d < EPS) continue;
            }

            double force = f_rep(d, fdl->k);
            double ux = dx / d;
            double uy = dy / d;

            fdl->dis[v].first  += ux * force;
            fdl->dis[v].second += uy * force;
        }
    }

    // attractive forces
    std::vector<std::vector<node_int>> adj_matrix = graph->get_adj_matrix();
    for(node_int v = 0; v < (node_int)n; ++v){
        for(node_int u : adj_matrix[v]) {
            if (u == v) continue;
            double dx = fdl->pos[v].first  - fdl->pos[u].first;
            double dy = fdl->pos[v].second - fdl->pos[u].second;
            double d  = length(dx, dy);

            if(d < EPS) {
                dx = ((double)std::rand() / RAND_MAX - 0.5) * 1e-3;
                dy = ((double)std::rand() / RAND_MAX - 0.5) * 1e-3;
                d = length(dx, dy);
                if (d < EPS) continue;
            }

            double force = f_att(d, fdl->k) / 2.0; // undirected edges counted twice
            double ux = dx / d;
            double uy = dy / d;

            fdl->dis[v].first  -= ux * force;
            fdl->dis[v].second -= uy * force;
            fdl->dis[u].first  += ux * force;
            fdl->dis[u].second += uy * force;
        }
    }

    // quadratic gravity toward the center
    const double gravity_strength = fdl::GRAVITY_STRENGTH;
    for(node_int v = 0; v < n; ++v){
        fdl->dis[v].first  += -fdl->pos[v].first  * gravity_strength;
        fdl->dis[v].second += -fdl->pos[v].second * gravity_strength;
    }

    // apply displacements
    for(node_int v = 0; v < n; ++v){
        double dx = fdl->dis[v].first;
        double dy = fdl->dis[v].second;
        double disp_len = length(dx, dy);
        if(disp_len < 1e-12) continue;

        double limited = std::min(disp_len, fdl->temp);
        double ux = dx / disp_len;
        double uy = dy / disp_len;

        fdl->pos[v].first  += ux * limited;
        fdl->pos[v].second += uy * limited;
    }

    // recenter layout so centroid stays at (0,0)
    double cx = 0.0, cy = 0.0;
    for (node_int v = 0; v < n; ++v) {
        cx += fdl->pos[v].first;
        cy += fdl->pos[v].second;
    }
    cx /= n;
    cy /= n;
    for (node_int v = 0; v < n; ++v) {
        fdl->pos[v].first  -= cx;
        fdl->pos[v].second -= cy;
    }

    // cool down
    fdl->temp = fdl::FDL_START_TEMP * (1.0 - (double)iteration / (double)fdl::FDL_MAX_ITER);
}


/*
FDL *fdl_start(Graph* graph){
    node_int node_count = graph->get_vertex_nr();
    std::vector<std::pair<double,double>> pos(node_count);
    std::vector<std::pair<double,double>> dis(node_count, {0,0});
    std::vector<std::vector<node_int>> adj_matrix = graph->get_adj_matrix();
    
    for(int i = 0; i < node_count; i++){
        pos[i] = {std::rand() % fdl::HEIGHT, std::rand() % fdl::WIDTH};
    }

    const int area = fdl::HEIGHT * fdl::WIDTH;
    const double k = std::sqrt(area/graph->get_vertex_nr());

    FDL *fdl = new FDL(pos, dis, adj_matrix, graph, fdl::WIDTH, fdl::HEIGHT, fdl::FDL_MAX_ITER, area, k, fdl::FDL_START_TEMP);
    return fdl;
}
*/

FDL *fdl_start(Graph* graph){
    node_int node_count = graph->get_vertex_nr();
    std::vector<std::pair<double,double>> pos(node_count);
    std::vector<std::pair<double,double>> dis(node_count, {0.0,0.0});
    std::vector<std::vector<node_int>> adj_matrix = graph->get_adj_matrix();

    // use doubles and uniform distribution
    std::mt19937 rng((unsigned)std::time(nullptr));
    std::uniform_real_distribution<double> rx(-fdl::WIDTH/2.0, fdl::WIDTH/2.0);
    std::uniform_real_distribution<double> ry(-fdl::HEIGHT/2.0, fdl::HEIGHT/2.0);

    for(int i = 0; i < node_count; i++){
        pos[i] = { rx(rng), ry(rng) }; // x,width ; y,height
    }

    const double area = (double)fdl::WIDTH * (double)fdl::HEIGHT;
    const double k = std::sqrt(area / (double)node_count); // NOTE: node_count not edge count

    FDL *fdl = new FDL(pos, dis, adj_matrix, graph, fdl::WIDTH, fdl::HEIGHT, (int)area, fdl::FDL_MAX_ITER, k, fdl::FDL_START_TEMP);
    return fdl;
}

void fdl_to_json(std::string file_name, Graph* graph, FDL* fdl) {
    std::string out_name = file_name.substr(0, file_name.size() - 4) + "-fdl.json";
    DEBUG_PRINT("Creating JSON: " + out_name);

    std::ofstream out;
    out.open(out_name);
    if (!out.is_open()) {
        std::cerr << "[ERROR] could not open " << out_name << " for writing\n";
        return;
    }

    std::vector<node_int> ranking;
    if(fdl::INCLUDE_RANK_JSON){
        ranking = rank_graph(graph, (ranking_algorithm)config::RANKING_ALGORITHM);
    }

    out << "{\n";

    // write nodes
    out << "  \"nodes\": [\n";
    for (node_int v = 0; v < graph->get_vertex_nr(); v++) {
        // don't show if it's an isolated node
        if(graph->get_adj_matrix()[v].empty() && !fdl::SHOW_ISOLATED_NODES_JSON){
            continue;
        }

        out << "    {\"id\": " << v
            << ", \"x\": " << fdl->pos[v].first
            << ", \"y\": " << fdl->pos[v].second
            << ", \"label\": \"" << graph->get_communities()[v] << "\"";

        if(fdl::INCLUDE_NEIGHBOURS_JSON){
            node_int neighbour_number = graph->get_adj_matrix()[v].size();
            out << ", \"neighbours\": " << neighbour_number;
        }
        if(fdl::INCLUDE_RANK_JSON){
            out << ", \"rank\": " << ranking[v];
        }

        out << "}";
        if (v < graph->get_vertex_nr() - 1) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    // write edges
    out << "  \"edges\": [\n";
    for (node_int e = 0; e < graph->get_vertex_nr(); e++) {
        std::vector<node_int> neighbours = graph->get_adj_matrix()[e];
        for (size_t i = 0; i < neighbours.size(); i++) {
            out << "    {\"source\": " << e
                << ", \"target\": " << neighbours[i] << "}";
            if (!(e == graph->get_vertex_nr() - 1 && i == neighbours.size() - 1)) {
                out << ",";
            }
            out << "\n";
        }
    }
    out << "  ]\n";
    out << "}\n";

    out.close();

    DEBUG_PRINT("Created JSON: " + out_name);
}

/**
 * @brief Interface function that calls all necessary functions in order to get the graph in the right shape.
 */
void fdl_run(std::string file_name, Graph* graph){
    DEBUG_PRINT("FDL started");

    FDL *fdl = fdl_start(graph);

    fdl_to_json(file_name.substr(0, file_name.size() - 4) + std::to_string(0) + file_name.substr(file_name.size() - 4, file_name.size()), graph, fdl);
    fdl_iteration(fdl, graph, 0);
    for(int iteration = 1; iteration < fdl::FDL_MAX_ITER; iteration++){
        DEBUG_PRINT("FDL iteration: " + std::to_string(iteration));
        fdl_iteration(fdl, graph, iteration);
        //fdl_to_json(file_name.substr(0, file_name.size() - 4) + std::to_string(iteration) + file_name.substr(file_name.size() - 4, file_name.size()), graph, fdl);
    }
    fdl_to_json(file_name.substr(0, file_name.size() - 4) + std::to_string(1) + file_name.substr(file_name.size() - 4, file_name.size()), graph, fdl);


    DEBUG_PRINT("FDL exited");

    return;
}