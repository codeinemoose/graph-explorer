#include "main.h"
#include "preproc.h"
#include "force-directed-layout.h"
#include "preproc.h"

bool DEBUG_MODE;

void DEBUG_PRINT(std::string str){
    if(DEBUG_MODE){
        std::cout << "[DEBUG] " << str << std::endl;
    }
}

int main(int const argc, char* argv[]){
    std::string command;
    std::string arg;

    if(argc > 1){
        command = argv[1];
    }

    if(argc > 2){
        for(int i = 2; i < argc; i++){
            arg = argv[i];
            if(arg == "-d"){
                DEBUG_MODE = true;
            }
        }
    }
    
    Graph* graph;
    if(command == "process"){
        graph = preproc(argv[2], true);
        fdl_run(argv[2], graph);

        return 1;
    }

    return 0;
}