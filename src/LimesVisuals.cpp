#include "Config.hpp"
#include "TargetESP.hpp"

#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include <pl/Config.hpp>

#include <algorithm>
#include <filesystem>
#include <string>

class LimesVisuals {
public:
    static LimesVisuals& instance() {
        static LimesVisuals inst;
        return inst;
    }

    LimesVisuals() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load() {
        auto& self = getSelf();

        try {
            std::filesystem::create_directories(self.getConfigDir());
        } catch (...) {
            self.getLogger().warn("Could not create config directory");
        }

        mConfig = std::make_unique<pl::config::ConfigFile<ModConfig>>(
            ModConfig{},
            self.getConfigDir() / "config.json"
        );

        mConfig->load();

        auto& c = mConfig->value();
        c.intensity     = std::clamp(c.intensity, 0, 100);
        c.particleCount = std::clamp(c.particleCount, 1, 64);

        targetesp::applyConfig(c);
        self.getLogger().info("Lime's Visuals (TargetESP) loaded v0.2.0");
        return true;
    }

    bool enable() {
        auto& self = getSelf();
        const auto& cfg = mConfig->value();
        targetesp::applyConfig(cfg);

        if (!targetesp::install()) {
            self.getLogger().error("TargetESP hooks failed – menu still available");
        }

        auto builder = pl::modmenu::ModuleBuilder("limes_visuals.targetesp", "Lime's Visuals – TargetESP")
            .modId(self.getId())
            .description("Hit particle styles (TargetESP)")
            .defaultEnabled(cfg.masterEnabled)
            .onToggle([this](std::string_view, bool enabled) {
                mConfig->value().masterEnabled = enabled;
                targetesp::applyConfig(mConfig->value());
                mConfig->save();
            });

        auto addStyle = [&](const char* key, const char* label, bool value) {
            builder.config(key, label, pl::modmenu::ConfigType::Toggle, value ? "true" : "false");
        };

        addStyle("styleNova",             "Nova",               cfg.styleNova);
        addStyle("styleWild",             "Wild",               cfg.styleWild);
        addStyle("styleNursultanDiamond", "Nursultan Diamond",  cfg.styleNursultanDiamond);
        addStyle("styleNursultanScanner", "Nursultan Scanner",  cfg.styleNursultanScanner);
        addStyle("styleGhostsClient",     "Ghosts (Client)",    cfg.styleGhostsClient);
        addStyle("styleGhostsCatlean",    "Ghosts (Catlean)",   cfg.styleGhostsCatlean);
        addStyle("styleWexsideSwords",    "Wexside Swords",     cfg.styleWexsideSwords);
        addStyle("styleSimsCrystal",      "Sims Crystal",       cfg.styleSimsCrystal);
        addStyle("styleJello",            "Jello",              cfg.styleJello);
        addStyle("styleRockstarCrystals", "Rockstar Crystals",  cfg.styleRockstarCrystals);
        addStyle("styleRockstarChains",   "Rockstar Chains",    cfg.styleRockstarChains);
        addStyle("styleHitBubble",        "HitBubble",          cfg.styleHitBubble);

        builder
            .config("onlyPlayers", "Only Players",
                    pl::modmenu::ConfigType::Toggle,
                    cfg.onlyPlayers ? "true" : "false")
            .config("onlyWhenIHit", "Only When I Hit",
                    pl::modmenu::ConfigType::Toggle,
                    cfg.onlyWhenIHit ? "true" : "false")
            .config("intensity", "Intensity",
                    pl::modmenu::ConfigType::SliderInt,
                    std::to_string(cfg.intensity), "0", "100")
            .config("particleCount", "Particle Count",
                    pl::modmenu::ConfigType::SliderInt,
                    std::to_string(cfg.particleCount), "1", "64")
            .onConfigChanged([this](std::string_view, std::string_view key, std::string_view value) {
                auto& c = mConfig->value();
                auto asBool = [](std::string_view v) {
                    return v == "true" || v == "1";
                };
                try {
                    if      (key == "styleNova")              c.styleNova = asBool(value);
                    else if (key == "styleWild")              c.styleWild = asBool(value);
                    else if (key == "styleNursultanDiamond")  c.styleNursultanDiamond = asBool(value);
                    else if (key == "styleNursultanScanner")  c.styleNursultanScanner = asBool(value);
                    else if (key == "styleGhostsClient")      c.styleGhostsClient = asBool(value);
                    else if (key == "styleGhostsCatlean")     c.styleGhostsCatlean = asBool(value);
                    else if (key == "styleWexsideSwords")     c.styleWexsideSwords = asBool(value);
                    else if (key == "styleSimsCrystal")       c.styleSimsCrystal = asBool(value);
                    else if (key == "styleJello")             c.styleJello = asBool(value);
                    else if (key == "styleRockstarCrystals")  c.styleRockstarCrystals = asBool(value);
                    else if (key == "styleRockstarChains")    c.styleRockstarChains = asBool(value);
                    else if (key == "styleHitBubble")         c.styleHitBubble = asBool(value);
                    else if (key == "onlyPlayers")            c.onlyPlayers = asBool(value);
                    else if (key == "onlyWhenIHit")           c.onlyWhenIHit = asBool(value);
                    else if (key == "intensity")              c.intensity = std::clamp(std::stoi(std::string(value)), 0, 100);
                    else if (key == "particleCount")          c.particleCount = std::clamp(std::stoi(std::string(value)), 1, 64);
                } catch (...) {}
                targetesp::applyConfig(c);
                mConfig->save();
            })
            .registerModule();

        self.getLogger().info("Lime's Visuals (TargetESP) enabled");
        return true;
    }

    bool disable() {
        targetesp::uninstall();
        getSelf().getLogger().info("Lime's Visuals disabled");
        return true;
    }

    bool unload() {
        targetesp::uninstall();
        mConfig.reset();
        getSelf().getLogger().info("Lime's Visuals unloaded");
        return true;
    }

private:
    ll::mod::NativeMod& mSelf;
    std::unique_ptr<pl::config::ConfigFile<ModConfig>> mConfig;
};

PL_REGISTER_MOD(LimesVisuals, LimesVisuals::instance())
