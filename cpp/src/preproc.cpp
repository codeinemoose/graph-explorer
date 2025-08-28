#include "preproc.h"
#include "graph.h"
#include "config.h"
#include "labelprop.h"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <filesystem>

Graph* txt_to_graph(std::string dir){
    std::ifstream in_file(dir);
    
    std::vector<std::pair<node_int, node_int>> edges;
    std::string line;
    node_int idA;
    node_int idB;
    node_int vertex_nr;
    //node_int edge_nr;
    
    //edge_nr = 0;
    vertex_nr = 0;
    while(std::getline(in_file, line)){
        std::istringstream iss(line);
        iss >> idA >> idB;

        edges.push_back({idA, idB});
        edges.push_back({idB, idA}); // for undirected
        vertex_nr = std::max(vertex_nr, std::max(idA, idB));
    }
    in_file.close();
    
    vertex_nr++;
    
    std::vector<node_int> degrees(vertex_nr, 0);
    for(auto &e : edges){
        degrees[e.first]++;
    }

    std::vector<node_int> offsets(vertex_nr + 1, 0);
    for(node_int i = 0; i < vertex_nr; i++){
        offsets[i + 1] = offsets[i] + degrees[i];
    }

    std::vector<node_int> writepos(vertex_nr, 0);
    std::vector<node_int> targets(offsets[vertex_nr]);
    for(auto &e : edges){
        idA = e.first;
        idB = e.second;
        targets[offsets[idA] + writepos[idA]] = idB;
        
        writepos[idA]++;
    }

    std::vector<node_int> communities(vertex_nr);
    for(node_int i = 0; i < vertex_nr; i++){
        communities[i] = i;
    }

    std::cout << "offsets[vertex_nr] (edges): " << offsets[vertex_nr] << std::endl;
    std::cout << "vertex_nr: " << vertex_nr << std::endl;

    Graph* graph = new Graph(offsets[vertex_nr], vertex_nr, offsets, targets, degrees, communities);

    return graph;
}

/**
 * @brief Creates the binary CSR from the Graph class. It uses the format:
 *      
 *      [[Type (8 bits)]-[Version (8 bits)]-[Node count (64 bits)]-[Edge count (64 bits)]] <- Header
 *      [[Offsets]-[Targets]] <- Payload
 */
int graph_to_bin(std::string file_name, Graph* graph){

    std::ofstream file;
    file.open(file_name.substr(0, file_name.size() - 4)+ "-graph.bin", std::ios::binary);
    std::cout << "opened: " << file_name << std::endl;

    uint8_t type_block = 0x00;
    uint8_t version_block = 0x00;
    node_int node_count_block = graph->get_vertex_nr();
    node_int edge_count_block = graph->get_edge_nr();
    std::vector<node_int> offsets_vector = graph->get_offsets();
    std::vector<node_int> targets_vector = graph->get_targets();

    node_int offset_size = node_count_block + 1;
    node_int targets_size = edge_count_block;

    
    node_int header_size = (2 * sizeof(uint8_t)) + (2 * sizeof(node_int));
    node_int payload_size = offset_size * sizeof(node_int) + targets_size * sizeof(node_int);
    
    node_int file_size = header_size + payload_size;
    
    char* data = new char[file_size];
    // Insert header
    data[0] = type_block;
    data[1] = version_block;
    memcpy(data + 2, &node_count_block, sizeof(node_int));
    memcpy(data + 2 + sizeof(node_int), &edge_count_block, sizeof(node_int));
    // Insert payload
    for (node_int i = 0; i < offset_size; i++) {
        memcpy(data + header_size + i * sizeof(node_int), &offsets_vector[i], sizeof(node_int));
    }
    for (node_int i = 0; i < targets_size; i++) {
        memcpy(data + header_size + offset_size * sizeof(node_int) + i * sizeof(node_int), &targets_vector[i], sizeof(node_int));
    }


    file.write(data, file_size);

    file.close();
    std::cout << "closed: " << file_name << std::endl;
    delete[] data;
    return 1;
}

/**
 * @brief Creates a binary for the communities.
 */
int communities_to_bin(std::string file_name, Graph* graph, int iteration){
    std::ofstream file;
    file.open(file_name.substr(0, file_name.size() - 4)+ "-communities-" + std::to_string(iteration) + ".bin", std::ios::binary);
    std::vector<node_int> communities = graph->get_communities();

    node_int node_count = graph->get_vertex_nr();
    char* data = new char[node_count * sizeof(node_int)];

    for(node_int i = 0; i < node_count; i++){
        memcpy(data + i * sizeof(node_int), &communities[i], sizeof(node_int));
    }

    file.write(data, node_count * sizeof(node_int));

    file.close();
    delete[] data;
    return 1;
}

/**
 * @brief Preprocessor for the graph. This function turns a simple textfile (txt) into a binary CSR.
 * 
 * @param       dir         The directory of the file we want to convert 
 */
Graph* preproc(std::string dir, bool communities){
    Graph* graph = txt_to_graph(dir);
    
    if(communities){
        communities_to_bin(dir, graph, 0);
        for(uint64_t iteration = 1; iteration < config::MAX_PROP_ITER; iteration++){
            label_prop(graph);
            //communities_to_bin(dir, graph, iteration);            
        }
    }
    communities_to_bin(dir, graph, 0);

    graph_to_bin(dir, graph);

    return graph;
}