#pragma once

#include <cstdint>

struct ModConfig {
    bool    enabled      = true;
    bool    throughWalls = true;
    int     colorR       = 255;   // default orange
    int     colorG       = 140;
    int     colorB       = 0;
    int     colorA       = 200;
};
