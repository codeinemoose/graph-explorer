/**
 * @brief Implements Fruchtermann and Reingold. (see: https://cs.brown.edu/people/rtamassi/gdhandbook/chapters/force-directed.pdf)
 */
#include "graph.h"
#include "force-directed-layout.h"
#include "main.h"
#include <math.h>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>

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
double dis(double a, double b){
    return std::abs(a - b);
}

double cool(double temp){
    return temp/2;
}

/*
void fdl_iteration(FDL *fdl, Graph* graph){
    double delta_first;
    double delta_second;

    // calculate repulsive forces
    for(node_int v = 0; v < graph->get_vertex_nr(); v++){
        fdl->dis[v] = {0,0};
        for(node_int u = 0; u < graph->get_vertex_nr(); u++){
            if(u == v) continue;

            delta_first         = fdl->pos[v].first     - fdl->pos[u].first;
            delta_second        = fdl->pos[v].second    - fdl->pos[u].second;

            fdl->dis[v] = {fdl->dis[v].first + ((delta_first/std::abs(delta_first)) * f_rep(std::abs(delta_first), fdl->k)),
                             fdl->dis[v].second + ((delta_second/std::abs(delta_second)) * f_rep(std::abs(delta_second), fdl->k))};

            DEBUG_PRINT("dis[" + std::to_string(v) + "] " + "{" + std::to_string(fdl->dis[v].first) + ", " + std::to_string(fdl->dis[v].second) + "}");
        }
    }

    // calculate attractive forces
    std::vector<std::vector<node_int>> adj_matrix = graph->get_adj_matrix();
    for(node_int v = 0; v < graph->get_vertex_nr(); v++){
        for(node_int u : adj_matrix[v]){
            delta_first         = fdl->pos[v].first     - fdl->pos[u].first;
            delta_second        = fdl->pos[v].second    - fdl->pos[u].second;

            fdl->dis[v].first   = fdl->dis[v].first     - (delta_first/std::abs(delta_first))   * f_att(std::abs(delta_first), fdl->k);
            fdl->dis[v].second  = fdl->dis[v].second    - (delta_second/std::abs(delta_second)) * f_att(std::abs(delta_second), fdl->k);

            fdl->dis[u].first   = fdl->dis[u].first     - (delta_first/std::abs(delta_first))   * f_att(std::abs(delta_first), fdl->k);
            fdl->dis[u].second  = fdl->dis[u].second    - (delta_second/std::abs(delta_second)) * f_att(std::abs(delta_second), fdl->k);
        }
    }

    // limit max displacement to temperature t and prevent from displacement outside frame
    for(node_int v = 0; v < graph->get_vertex_nr(); v++){
        fdl->pos[v].first   = fdl->pos[v].first     + (fdl->dis[v].first/std::abs(fdl->dis[v].first))   * std::min(fdl->dis[v].first, fdl->temp);
        fdl->pos[v].second  = fdl->pos[v].second    + (fdl->dis[v].second/std::abs(fdl->dis[v].second)) * std::min(fdl->dis[v].second, fdl->temp);

        fdl->pos[v].first   = std::min((double)fdl->height/2, std::max((double)-(fdl->height/2), fdl->pos[v].first));
        fdl->pos[v].second  = std::min((double)fdl->width/2, std::max((double)-(fdl->width/2), fdl->pos[v].second));
    }
    
    // cool the fdl
    fdl->temp = cool(fdl->temp);

    return;
}
*/

static inline double length(double dx, double dy) {
    return std::hypot(dx, dy);
}

void fdl_iteration(FDL *fdl, Graph* graph){
    const double EPS = 1e-9;
    const size_t n = graph->get_vertex_nr();

    // zero displacements
    for(size_t v = 0; v < n; ++v) {
        fdl->dis[v] = {0.0, 0.0};
    }

    // repulsive forces (pairwise)
    for(size_t v = 0; v < n; ++v){
        for(size_t u = 0; u < n; ++u){
            if(u == v) continue;

            double dx = fdl->pos[v].first  - fdl->pos[u].first;
            double dy = fdl->pos[v].second - fdl->pos[u].second;
            double d  = length(dx, dy);

            if(d < EPS) {
                // jitter to avoid exact overlap
                dx = ((double)std::rand() / RAND_MAX - 0.5) * 1e-3;
                dy = ((double)std::rand() / RAND_MAX - 0.5) * 1e-3;
                d = length(dx, dy);
                if (d < EPS) continue;
            }

            double force = f_rep(d, fdl->k); // scalar
            double ux = dx / d;
            double uy = dy / d;
            fdl->dis[v].first  += ux * force;
            fdl->dis[v].second += uy * force;
        }
    }

    // attractive forces along edges
    std::vector<std::vector<node_int>> adj_matrix = graph->get_adj_matrix();
    for(node_int v = 0; v < (node_int)n; ++v){
        for(node_int u : adj_matrix[v]) {
            if (u == v) continue;
            double dx = fdl->pos[v].first  - fdl->pos[u].first;
            double dy = fdl->pos[v].second - fdl->pos[u].second;
            double d  = length(dx, dy);
            if(d < EPS) {
                // tiny jitter
                dx = ((double)std::rand() / RAND_MAX - 0.5) * 1e-3;
                dy = ((double)std::rand() / RAND_MAX - 0.5) * 1e-3;
                d = length(dx, dy);
                if (d < EPS) continue;
            }

            double force = f_att(d, fdl->k);
            double ux = dx / d;
            double uy = dy / d;

            // v gets -, u gets + as in pseudocode
            fdl->dis[v].first  -= ux * force;
            fdl->dis[v].second -= uy * force;

            fdl->dis[u].first  += ux * force;
            fdl->dis[u].second += uy * force;
        }
    }

    // limit displacement to temperature and update positions
    for(size_t v = 0; v < n; ++v){
        double dx = fdl->dis[v].first;
        double dy = fdl->dis[v].second;
        double disp_len = length(dx, dy);
        if(disp_len < 1e-12) continue;

        double limited = std::min(disp_len, fdl->temp);
        double ux = dx / disp_len;
        double uy = dy / disp_len;

        fdl->pos[v].first  += ux * limited;
        fdl->pos[v].second += uy * limited;

        // clamp to frame: assume pos.first = x (width), pos.second = y (height)
        fdl->pos[v].first  = std::min(fdl->width  / 2.0, std::max(-fdl->width  / 2.0, fdl->pos[v].first));
        fdl->pos[v].second = std::min(fdl->height / 2.0, std::max(-fdl->height / 2.0, fdl->pos[v].second));
    }

    // cool
    fdl->temp = cool(fdl->temp);
}


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

void fdl_to_json(std::string file_name, Graph* graph, FDL* fdl) {
    std::string out_name = file_name.substr(0, file_name.size() - 4) + "-fdl.json";
    DEBUG_PRINT("Creating JSON: " + out_name);

    std::ofstream out;
    out.open(out_name);
    if (!out.is_open()) {
        std::cerr << "[ERROR] could not open " << out_name << " for writing\n";
        return;
    }

    out << "{\n";

    // write nodes
    out << "  \"nodes\": [\n";
    for (node_int v = 0; v < graph->get_vertex_nr(); v++) {
        out << "    {\"id\": " << v
            << ", \"x\": " << fdl->pos[v].first
            << ", \"y\": " << fdl->pos[v].second
            << ", \"label\": \"" << graph->get_communities()[v] << "\"}";
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
    fdl_iteration(fdl, graph);

    for(int iteration = 1; iteration < fdl::FDL_MAX_ITER; iteration++){
        DEBUG_PRINT("FDL iteration: " + std::to_string(iteration));
        fdl_iteration(fdl, graph);
        fdl_to_json(file_name.substr(0, file_name.size() - 4) + std::to_string(iteration) + file_name.substr(file_name.size() - 4, file_name.size()), graph, fdl);
    }


    DEBUG_PRINT("FDL exited");

    return;
}