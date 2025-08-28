#ifndef CONFIG_H
#define CONFIG_H

extern bool DEBUG_MODE;

namespace config{
    constexpr int MAX_PROP_ITER = 30;
    constexpr int PROP_STEPS_PER_ITER = 1;

    // Ranking stuff
    /**
     * @brief The ranking algorithm currently selected.
     */
    constexpr int RANKING_ALGORITHM = 2;
    /**
     * @brief The amount of iterations we want the ranking algorithm to go through. Only affects algorithms which
     * go through more than one iteration. (e.g. NEIGHBOURHOOD is not affected)
     */
    constexpr int RANKING_ITERATIONS = 10;
}

namespace fdl{
    constexpr bool SHOW_ISOLATED_NODES_JSON = false;
    constexpr bool INCLUDE_NEIGHBOURS_JSON = true;
    constexpr bool INCLUDE_RANK_JSON = true;
    constexpr int FDL_MAX_ITER = 250;
    constexpr int WIDTH = 128;
    constexpr int HEIGHT = 128;
    constexpr double FDL_START_TEMP = fdl::HEIGHT/10;   // height/10 is just a heuristic, can be tweaked
    constexpr double GRAVITY_STRENGTH = 0.01;
}

#endif