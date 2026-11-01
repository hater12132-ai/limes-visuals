#pragma once

#include <cstdint>

struct ModConfig {
    // Required by pl::config::ConfigFile / TypedConfig concept
    int     version      = 1;

    bool    enabled      = true;
    bool    throughWalls = true;
    int     colorR       = 255;   // default orange
    int     colorG       = 140;
    int     colorB       = 0;
    int     colorA       = 200;
};
