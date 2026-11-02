#pragma once

#include "Config.hpp"
#include <atomic>
#include <cstdint>

namespace targetesp {

struct RuntimeState {
    std::atomic<bool> masterEnabled{true};
    std::atomic<bool> styleNova{true};
    std::atomic<bool> styleWild{false};
    std::atomic<bool> styleNursultanDiamond{false};
    std::atomic<bool> styleNursultanScanner{false};
    std::atomic<bool> styleGhostsClient{false};
    std::atomic<bool> styleGhostsCatlean{false};
    std::atomic<bool> styleWexsideSwords{false};
    std::atomic<bool> styleSimsCrystal{false};
    std::atomic<bool> styleJello{false};
    std::atomic<bool> styleRockstarCrystals{false};
    std::atomic<bool> styleRockstarChains{false};
    std::atomic<bool> styleHitBubble{false};
    std::atomic<bool> onlyPlayers{true};
    std::atomic<bool> onlyWhenIHit{true};
    std::atomic<int>  intensity{100};
    std::atomic<int>  particleCount{12};
};

RuntimeState& state();
void applyConfig(const ModConfig& cfg);
bool install();
void uninstall();
void onEntityHit(float x, float y, float z, bool isPlayer);

} // namespace targetesp
