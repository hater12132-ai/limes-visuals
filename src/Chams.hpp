#pragma once

#include "Config.hpp"
#include <atomic>
#include <cstdint>

namespace chams {

struct State {
    std::atomic<bool> enabled{true};
    std::atomic<bool> throughWalls{true};
    std::atomic<int>  colorR{255};
    std::atomic<int>  colorG{140};
    std::atomic<int>  colorB{0};
    std::atomic<int>  colorA{200};
};

State& state();
bool install();
void uninstall();
void applyConfig(const ModConfig& cfg);

} // namespace chams
