#include "Chams.hpp"

#include <pl/memory/Hook.hpp>

#include <android/log.h>
#include <dlfcn.h>
#include <GLES3/gl3.h>

#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#define LOG_TAG "LimesVisuals"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace chams {

namespace {

State g_state;

using glUniform4f_t          = void  (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
using glUniform4fv_t         = void  (*)(GLint, GLsizei, const GLfloat*);
using glGetUniformLocation_t = GLint (*)(GLuint, const GLchar*);
using glUseProgram_t         = void  (*)(GLuint);
using glGetActiveUniform_t   = void  (*)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
using glGetProgramiv_t       = void  (*)(GLuint, GLenum, GLint*);

glUniform4f_t          o_glUniform4f          = nullptr;
glUniform4fv_t         o_glUniform4fv         = nullptr;
glGetUniformLocation_t o_glGetUniformLocation = nullptr;
glUseProgram_t         o_glUseProgram         = nullptr;
glGetActiveUniform_t   o_glGetActiveUniform   = nullptr;
glGetProgramiv_t       o_glGetProgramiv       = nullptr;

std::mutex                        g_mutex;
std::unordered_map<GLuint, GLint> g_overlayLocations;
std::unordered_set<GLuint>        g_scanning;
thread_local GLuint               t_currentProgram = 0;
std::atomic<bool>                 g_installed{false};

bool isOverlayUniform(const char* name) {
    if (!name || !*name) return false;

    static const char* exact[] = {
        "OverlayColor", "ChangeColor", "OVERLAY_COLOR", "overlayColor",
        "changeColor", "u_OverlayColor", "u_ChangeColor", "ColorOverlay",
        "ENTITY_COLOR", "EntityColor", "overlay", nullptr
    };

    for (int i = 0; exact[i]; ++i) {
        if (strcasecmp(name, exact[i]) == 0)
            return true;
    }

    std::string lower(name);
    for (char& c : lower) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));

    const bool hasOverlay = lower.find("overlay") != std::string::npos;
    const bool hasChange  = lower.find("change")  != std::string::npos;
    const bool hasColor   = lower.find("color")   != std::string::npos;

    return (hasOverlay && hasColor) || (hasChange && hasColor);
}

void scanProgramUniforms(GLuint program) {
    if (program == 0) return;
    if (!o_glGetProgramiv || !o_glGetActiveUniform || !o_glGetUniformLocation)
        return;

    {
        std::lock_guard lock(g_mutex);
        if (g_overlayLocations.count(program) || g_scanning.count(program))
            return;
        g_scanning.insert(program);
    }

    GLint uniformCount = 0;
    o_glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

    GLint foundLoc = -1;
    char  nameBuf[256]{};

    for (GLint i = 0; i < uniformCount; ++i) {
        GLsizei length = 0;
        GLint   size   = 0;
        GLenum  type   = GL_NONE;

        o_glGetActiveUniform(program, static_cast<GLuint>(i),
                             sizeof(nameBuf) - 1, &length, &size, &type, nameBuf);
        nameBuf[sizeof(nameBuf) - 1] = '\0';

        if (length <= 0) continue;

        if (isOverlayUniform(nameBuf)) {
            GLint loc = o_glGetUniformLocation(program, nameBuf);
            if (loc >= 0) {
                foundLoc = loc;
                LOGI("Found overlay uniform '%s' @ %d (program %u)", nameBuf, loc, program);
                break;
            }
        }
    }

    {
        std::lock_guard lock(g_mutex);
        g_overlayLocations[program] = foundLoc;
        g_scanning.erase(program);
    }
}

void hk_glUseProgram(GLuint program) {
    t_currentProgram = program;
    if (!o_glUseProgram) return;

    if (program != 0 && g_state.enabled.load(std::memory_order_relaxed)) {
        bool needScan = false;
        {
            std::lock_guard lock(g_mutex);
            needScan = (g_overlayLocations.find(program) == g_overlayLocations.end());
        }
        if (needScan)
            scanProgramUniforms(program);
    }

    o_glUseProgram(program);
}

void hk_glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    if (!o_glUniform4f) return;

    if (g_state.enabled.load(std::memory_order_relaxed) && location >= 0) {
        GLint targetLoc = -1;
        {
            std::lock_guard lock(g_mutex);
            auto it = g_overlayLocations.find(t_currentProgram);
            if (it != g_overlayLocations.end())
                targetLoc = it->second;
        }

        if (targetLoc >= 0 && targetLoc == location) {
            const float r = g_state.colorR.load(std::memory_order_relaxed) / 255.0f;
            const float g = g_state.colorG.load(std::memory_order_relaxed) / 255.0f;
            const float b = g_state.colorB.load(std::memory_order_relaxed) / 255.0f;
            const float a = g_state.colorA.load(std::memory_order_relaxed) / 255.0f;
            o_glUniform4f(location, r, g, b, a);
            return;
        }
    }

    o_glUniform4f(location, v0, v1, v2, v3);
}

void hk_glUniform4fv(GLint location, GLsizei count, const GLfloat* value) {
    if (!o_glUniform4fv) return;

    if (g_state.enabled.load(std::memory_order_relaxed) && location >= 0 && count > 0 && value) {
        GLint targetLoc = -1;
        {
            std::lock_guard lock(g_mutex);
            auto it = g_overlayLocations.find(t_currentProgram);
            if (it != g_overlayLocations.end())
                targetLoc = it->second;
        }

        if (targetLoc >= 0 && targetLoc == location) {
            const float color[4] = {
                g_state.colorR.load(std::memory_order_relaxed) / 255.0f,
                g_state.colorG.load(std::memory_order_relaxed) / 255.0f,
                g_state.colorB.load(std::memory_order_relaxed) / 255.0f,
                g_state.colorA.load(std::memory_order_relaxed) / 255.0f
            };
            o_glUniform4fv(location, 1, color);
            return;
        }
    }

    o_glUniform4fv(location, count, value);
}

void* resolveGL(const char* name) {
    static void* handle = nullptr;
    if (!handle) {
        handle = dlopen("libGLESv3.so", RTLD_NOW | RTLD_NOLOAD);
        if (!handle) handle = dlopen("libGLESv2.so", RTLD_NOW | RTLD_NOLOAD);
        if (!handle) handle = dlopen("libGLESv3.so", RTLD_NOW);
        if (!handle) handle = dlopen("libGLESv2.so", RTLD_NOW);
    }
    if (!handle) {
        LOGE("Could not open libGLESv3/libGLESv2");
        return nullptr;
    }
    void* sym = dlsym(handle, name);
    if (!sym)
        LOGW("dlsym(%s) failed", name);
    return sym;
}

pl::memory::HookHandle g_hookUseProgram;
pl::memory::HookHandle g_hookUniform4f;
pl::memory::HookHandle g_hookUniform4fv;

} // anonymous namespace

State& state() { return g_state; }

void applyConfig(const ModConfig& cfg) {
    g_state.enabled.store(cfg.enabled, std::memory_order_relaxed);
    g_state.throughWalls.store(cfg.throughWalls, std::memory_order_relaxed);
    g_state.colorR.store(cfg.colorR, std::memory_order_relaxed);
    g_state.colorG.store(cfg.colorG, std::memory_order_relaxed);
    g_state.colorB.store(cfg.colorB, std::memory_order_relaxed);
    g_state.colorA.store(cfg.colorA, std::memory_order_relaxed);
}

bool install() {
    if (g_installed.load(std::memory_order_acquire)) {
        LOGW("Chams already installed");
        return true;
    }

    LOGI("Installing Chams hooks...");

    auto* pUseProgram = resolveGL("glUseProgram");
    auto* pUniform4f  = resolveGL("glUniform4f");
    auto* pUniform4fv = resolveGL("glUniform4fv");
    auto* pGetLoc     = resolveGL("glGetUniformLocation");
    auto* pGetActive  = resolveGL("glGetActiveUniform");
    auto* pGetProg    = resolveGL("glGetProgramiv");

    if (!pUseProgram || !pUniform4f || !pUniform4fv) {
        LOGE("Failed to resolve core GL functions – aborting install");
        return false;
    }

    o_glGetUniformLocation = reinterpret_cast<glGetUniformLocation_t>(pGetLoc);
    o_glGetActiveUniform   = reinterpret_cast<glGetActiveUniform_t>(pGetActive);
    o_glGetProgramiv       = reinterpret_cast<glGetProgramiv_t>(pGetProg);

    g_hookUseProgram = pl::memory::HookHandle(
        pUseProgram,
        reinterpret_cast<void*>(&hk_glUseProgram),
        reinterpret_cast<void**>(&o_glUseProgram)
    );

    g_hookUniform4f = pl::memory::HookHandle(
        pUniform4f,
        reinterpret_cast<void*>(&hk_glUniform4f),
        reinterpret_cast<void**>(&o_glUniform4f)
    );

    g_hookUniform4fv = pl::memory::HookHandle(
        pUniform4fv,
        reinterpret_cast<void*>(&hk_glUniform4fv),
        reinterpret_cast<void**>(&o_glUniform4fv)
    );

    const bool ok = g_hookUseProgram.installed() &&
                    g_hookUniform4f.installed()  &&
                    g_hookUniform4fv.installed();

    if (ok) {
        g_installed.store(true, std::memory_order_release);
        LOGI("Chams hooks installed successfully");
    } else {
        LOGE("One or more Chams hooks failed (UseProgram=%d Uniform4f=%d Uniform4fv=%d)",
             g_hookUseProgram.installed(),
             g_hookUniform4f.installed(),
             g_hookUniform4fv.installed());
        uninstall();
    }
    return ok;
}

void uninstall() {
    LOGI("Uninstalling Chams hooks...");
    g_hookUseProgram.reset();
    g_hookUniform4f.reset();
    g_hookUniform4fv.reset();

    {
        std::lock_guard lock(g_mutex);
        g_overlayLocations.clear();
        g_scanning.clear();
    }
    t_currentProgram = 0;
    g_installed.store(false, std::memory_order_release);
}

} // namespace chams
