/**
 * @brief Implements Fruchtermann and Reingold. (see: https://cs.brown.edu/people/rtamassi/gdhandbook/chapters/force-directed.pdf)
 */
#include "graph.h"
#include "force-directed-layout.h"
#include "main.h"
#include "config.h"

/**
 * @brief Implements the Quadtree used for improving the runtime of the FDL algorithm.
 */
class QTree{
    
};

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

            double force;
            if(graph->get_graph_type() == UNDIRECTED) {
                force = f_att(d, fdl->k) / 2.0; // undirected edges counted twice
            }
            else{
                force = f_att(d, fdl->k);
            }
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
    for(int iteration = 1; iteration <= fdl::FDL_MAX_ITER; iteration++){
        //DEBUG_PRINT("FDL iteration: " + std::to_string(iteration));
        print_progress_bar((double)iteration / fdl::FDL_MAX_ITER);
        fdl_iteration(fdl, graph, iteration);
        //fdl_to_json(file_name.substr(0, file_name.size() - 4) + std::to_string(iteration) + file_name.substr(file_name.size() - 4, file_name.size()), graph, fdl);
    }
    std::cout << std::endl;
    fdl_to_json(file_name.substr(0, file_name.size() - 4) + std::to_string(1) + file_name.substr(file_name.size() - 4, file_name.size()), graph, fdl);


    DEBUG_PRINT("FDL exited");

    return;
}

class quadtree{
    static constexpr int infty = std::numeric_limits<int>::infinity();
    static constexpr node_int nil = node_int(-1);
    
    // We store the points as integer x,y coordinates. This also means, that the canvas should ideally be a multiple of 2.
    struct point{
        int x,y;
    };
    
    // The bounding box of a node.
    struct aabb{
        point min{infty, infty};
        point max{-infty, -infty};
        
        aabb& operator |= (const point& p){
            min.x = std::min(min.x, p.x);
            min.y = std::min(min.y, p.y);
            max.x = std::max(max.x, p.x);
            max.y = std::max(max.y, p.y);
            
            return *this;
        }
    };
    
    point middle(const point a, const point b){
        return {(a.x + b.x) / 2, (a.y + b.y) / 2};
    }
    
    template<typename T>aabb bound(T begin, T end){
        aabb return_me;
        for(auto e = begin; e != end; e++){
            return_me |= *e;
        }

        return return_me;
    }

    struct node{
        node_int children[2][2]{
            {nil, nil},
            {nil, nil}
        };
    };

    struct qtree{
        aabb bound;
        node_int root;
        std::vector<node> nodes;
        std::vector<float> mass;
        std::vector<point> center_of_mass;

        std::vector<point> points;
        /**
         * Stores the per-node data. Here the points for 'id + 1' are stored directly after
         * the points for 'id'.
         */
        std::vector<node_int> node_points_begin;
    };

    /**
     * @brief Recurisvely called build fucntion.
     * @warning This function iterates over [begin, end) (half-open interval).
     */
    template<typename T> node_int build_imp(qtree& tree, const aabb& bound, T begin, T end, size_t depth_limit){
        // Tree is emtpy
        if(begin == end){
            return nil;
        }

        node_int return_me = tree.nodes.size();
        tree.nodes.emplace_back();

        if(begin + 1 == end){
            return return_me;
        }

        // We constrain the depth as to avoid infinite recursion.
        if(depth_limit == 0){
            return return_me;
        }

        point mid = middle(bound.min, bound.max);
        
        // Partition the points along the y axis, whether or not they're smaller than the mid point.
        T split_y = std::partition(begin, end, 
            // We use a lambda function to capture the predicates.
            [mid](const point& p){
                return p.y < mid.y;
        });

        // Partition the points along the x axis, whether or not they're greater or less than the mid point and split_y.
        T split_x_lower = std::partition(begin, split_y, 
            // We use a lambda function to capture the predicates.
            [mid](const point& p){
                return p.x < mid.x;
        });

        // Partition the points along the x axis, whether or not they're greater or less than the mid point and split_y.
        T split_x_upper = std::partition(split_y, end, 
            // We use a lambda function to capture the predicates.
            [mid](const point& p){
                return p.x < mid.x;
        });

        // Compute the starting point index when building a node.
        tree.node_points_begin[return_me] = (begin - tree.points.begin());

        // Recursively compute the points we want to add to the quadrants we just created.
        tree.nodes[return_me].children[0][0] = build_imp(tree, {bound.min, mid}, begin, split_x_lower, depth_limit - 1);
        tree.nodes[return_me].children[0][1] = build_imp(tree, {{mid.x, bound.min.y}, {bound.max.x, mid.y}}, split_x_lower, split_y, depth_limit - 1);
        tree.nodes[return_me].children[1][0] = build_imp(tree, {{bound.min.x, mid.y}, {mid.x, bound.max.y,}}, split_y, split_x_upper, depth_limit - 1);
        tree.nodes[return_me].children[1][1] = build_imp(tree, {mid, bound.max}, split_x_upper, end, depth_limit - 1);

        return return_me;
    }

    template<typename T> quadtree build(std::vector<point> points){
        qtree return_me;
        return_me.points = std::move(points);
        return_me.root = build_imp(return_me, bound(return_me.points.begin, return_me.points.end), return_me.points.begin, return_me.points.end, 
                                    config::MAX_QUADTREE_DEPTH);
        return_me.node_points_begin.push_back(return_me.points.size());
        return return_me;
    }
};