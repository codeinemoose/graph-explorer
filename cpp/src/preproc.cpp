#include "./include/preproc.h"
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

using namespace std;
vector<pair<int, int>> edges;

int preproc(string dir){
    ifstream in_file(dir);
    
    string line;
    int idA;
    int idB;
    int vertex_nr;
    int edge_nr;
    
    edge_nr = 0;
    vertex_nr = 0;
    while(getline(in_file, line)){
        istringstream iss(line);
        iss >> idA >> idB;
        edges.push_back({idA, idB});
        vertex_nr = max(vertex_nr, max(idA, idB));
    }
    in_file.close();
    
    vertex_nr++;
    
    vector<int> deg(vertex_nr, 0);
    for(auto &e : edges){
        deg[e.first]++;
    }

    vector<int> offsets(vertex_nr + 1, 0);
    for(int i = 0; i < vertex_nr; i++){
        offsets[i + 1] = offsets[i] + deg[i];
    }

    vector<int> writepos(vertex_nr, 0);
    vector<int> targets(offsets[vertex_nr]);
    for(auto &e : edges){
        idA = e.first;
        idB = e.second;
        targets[offsets[idA] + writepos[idA]] = idB;
        writepos[idA]++;
    }
}