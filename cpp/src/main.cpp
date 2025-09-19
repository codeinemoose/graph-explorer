#include <chrono>
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

void print_progress_bar(double progress){
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

int main(int const argc, char* argv[]){
    auto t1 = std::chrono::high_resolution_clock::now();

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
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    std::cout << "Ran for: " << ms_int.count() << "ms" << std::endl;
    return 1;
}