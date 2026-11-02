#pragma once

#include <cstdint>

struct ModConfig {
    int  version = 1;
    bool masterEnabled = true;
    bool styleNova = true;
    bool styleWild = false;
    bool styleNursultanDiamond = false;
    bool styleNursultanScanner = false;
    bool styleGhostsClient = false;
    bool styleGhostsCatlean = false;
    bool styleWexsideSwords = false;
    bool styleSimsCrystal = false;
    bool styleJello = false;
    bool styleRockstarCrystals = false;
    bool styleRockstarChains = false;
    bool styleHitBubble = false;
    bool onlyPlayers = true;
    bool onlyWhenIHit = true;
    int  intensity = 100;
    int  particleCount = 12;
};
