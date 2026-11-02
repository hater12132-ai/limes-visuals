#include "TargetESP.hpp"

#include <android/log.h>
#include <atomic>

#define LOG_TAG "LimesVisuals"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace targetesp {

namespace {
RuntimeState g_state;
std::atomic<bool> g_installed{false};
}

RuntimeState& state() { return g_state; }

void applyConfig(const ModConfig& cfg) {
    g_state.masterEnabled.store(cfg.masterEnabled);
    g_state.styleNova.store(cfg.styleNova);
    g_state.styleWild.store(cfg.styleWild);
    g_state.styleNursultanDiamond.store(cfg.styleNursultanDiamond);
    g_state.styleNursultanScanner.store(cfg.styleNursultanScanner);
    g_state.styleGhostsClient.store(cfg.styleGhostsClient);
    g_state.styleGhostsCatlean.store(cfg.styleGhostsCatlean);
    g_state.styleWexsideSwords.store(cfg.styleWexsideSwords);
    g_state.styleSimsCrystal.store(cfg.styleSimsCrystal);
    g_state.styleJello.store(cfg.styleJello);
    g_state.styleRockstarCrystals.store(cfg.styleRockstarCrystals);
    g_state.styleRockstarChains.store(cfg.styleRockstarChains);
    g_state.styleHitBubble.store(cfg.styleHitBubble);
    g_state.onlyPlayers.store(cfg.onlyPlayers);
    g_state.onlyWhenIHit.store(cfg.onlyWhenIHit);
    g_state.intensity.store(cfg.intensity);
    g_state.particleCount.store(cfg.particleCount);
}

void onEntityHit(float x, float y, float z, bool isPlayer) {
    if (!g_state.masterEnabled.load(std::memory_order_relaxed)) return;
    if (g_state.onlyPlayers.load(std::memory_order_relaxed) && !isPlayer) return;
    LOGI("TargetESP hit at (%.2f, %.2f, %.2f) isPlayer=%d", x, y, z, (int)isPlayer);
}

bool install() {
    if (g_installed.load(std::memory_order_acquire)) return true;
    LOGI("TargetESP install: hooks pending signatures for 1.26.60.4");
    g_installed.store(true, std::memory_order_release);
    return true;
}

void uninstall() {
    LOGI("TargetESP uninstall");
    g_installed.store(false, std::memory_order_release);
}

} // namespace targetesp
