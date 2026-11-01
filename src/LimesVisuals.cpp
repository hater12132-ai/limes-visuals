#include "Config.hpp"
#include "Chams.hpp"

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
            self.getConfigDir() / "config.json"
        );

        if (!mConfig->load()) {
            mConfig->value() = ModConfig{};
            mConfig->save();
        }

        auto& c = mConfig->value();
        c.colorR = std::clamp(c.colorR, 0, 255);
        c.colorG = std::clamp(c.colorG, 0, 255);
        c.colorB = std::clamp(c.colorB, 0, 255);
        c.colorA = std::clamp(c.colorA, 0, 255);

        chams::applyConfig(c);
        self.getLogger().info("Lime's Visuals loaded (v0.1.0)");
        return true;
    }

    bool enable() {
        auto& self = getSelf();
        const auto& cfg = mConfig->value();

        chams::applyConfig(cfg);

        if (!chams::install()) {
            self.getLogger().error("Chams hooks failed to install – menu will still work");
        }

        pl::modmenu::ModuleBuilder("limes_visuals.chams", "Lime's Visuals")
            .modId(self.getId())
            .description("Chams / through-wall entity coloring")
            .defaultEnabled(cfg.enabled)
            .onToggle([this](std::string_view, bool enabled) {
                mConfig->value().enabled = enabled;
                chams::applyConfig(mConfig->value());
                mConfig->save();
            })
            .config("throughWalls", "Through Walls",
                    pl::modmenu::ConfigType::Toggle,
                    cfg.throughWalls ? "true" : "false")
            .config("colorR", "Red",
                    pl::modmenu::ConfigType::SliderInt,
                    std::to_string(cfg.colorR), "0", "255")
            .config("colorG", "Green",
                    pl::modmenu::ConfigType::SliderInt,
                    std::to_string(cfg.colorG), "0", "255")
            .config("colorB", "Blue",
                    pl::modmenu::ConfigType::SliderInt,
                    std::to_string(cfg.colorB), "0", "255")
            .config("colorA", "Alpha",
                    pl::modmenu::ConfigType::SliderInt,
                    std::to_string(cfg.colorA), "0", "255")
            .onConfigChanged([this](std::string_view, std::string_view key, std::string_view value) {
                auto& c = mConfig->value();
                try {
                    if (key == "throughWalls") {
                        c.throughWalls = (value == "true" || value == "1");
                    } else if (key == "colorR") {
                        c.colorR = std::clamp(std::stoi(std::string(value)), 0, 255);
                    } else if (key == "colorG") {
                        c.colorG = std::clamp(std::stoi(std::string(value)), 0, 255);
                    } else if (key == "colorB") {
                        c.colorB = std::clamp(std::stoi(std::string(value)), 0, 255);
                    } else if (key == "colorA") {
                        c.colorA = std::clamp(std::stoi(std::string(value)), 0, 255);
                    }
                } catch (...) {}
                chams::applyConfig(c);
                mConfig->save();
            })
            .registerModule();

        self.getLogger().info("Lime's Visuals enabled");
        return true;
    }

    bool disable() {
        chams::uninstall();
        getSelf().getLogger().info("Lime's Visuals disabled");
        return true;
    }

    bool unload() {
        chams::uninstall();
        mConfig.reset();
        getSelf().getLogger().info("Lime's Visuals unloaded");
        return true;
    }

private:
    ll::mod::NativeMod& mSelf;
    std::unique_ptr<pl::config::ConfigFile<ModConfig>> mConfig;
};

PL_REGISTER_MOD(LimesVisuals, LimesVisuals::instance())
