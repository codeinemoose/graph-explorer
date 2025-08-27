#ifndef CONFIG_H
#define CONFIG_H

extern bool DEBUG_MODE;

namespace config{
    constexpr int MAX_PROP_ITER = 30;
    constexpr int PROP_STEPS_PER_ITER = 1;
}

namespace fdl{
    constexpr int FDL_MAX_ITER = 250;
    constexpr int WIDTH = 128;
    constexpr int HEIGHT = 128;
    constexpr double FDL_START_TEMP = fdl::HEIGHT/10;
    constexpr double GRAVITY_STRENGTH = 0.01;
}

#endif