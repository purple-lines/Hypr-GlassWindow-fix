#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/helpers/Color.hpp>
#include <regex>
#include <vector>
#include <string>
#include <any>

inline HANDLE PHANDLE = nullptr;

class CGlassWindow {
  public:
    static CGlassWindow& getInstance() {
        static CGlassWindow instance;
        return instance;
    }

    // Called once on plugin init
    void init(HANDLE pluginHandle) {
        m_pluginHandle = pluginHandle;
        registerConfig();
        reloadConfig();

        // Register render hook using the new dynamic callback API
        m_renderCallback = HyprlandAPI::registerCallbackDynamic(
            m_pluginHandle, "render",
            [this](void* thisptr, SCallbackInfo& info, std::any data) { 
                this->onRenderWindow(thisptr, info, data); 
            });

        // Register config reload hook
        m_configCallback = HyprlandAPI::registerCallbackDynamic(
            m_pluginHandle, "configReloaded",
            [this](void* thisptr, SCallbackInfo& info, std::any data) { 
                this->reloadConfig(); 
            });
    }

    // Called once on plugin exit
    void cleanup() {
        // Reset callback pointers to unregister them
        m_renderCallback.reset();
        m_configCallback.reset();
    }

  private:
    HANDLE m_pluginHandle = nullptr;

    // Callback handles - must be kept alive
    SP<HOOK_CALLBACK_FN> m_renderCallback;
    SP<HOOK_CALLBACK_FN> m_configCallback;

    // Config keys (strings, floats, ints)
    std::string m_rulesRaw;             // Raw regex/rule strings from config
    std::vector<std::regex> m_rules;    // Parsed regex rules

    float m_strength = 0.7f;            // Glass effect strength (blur etc)
    float m_chromaticAberration = 0.f;  // Chromatic aberration strength
    float m_opacity = 0.9f;             // Opacity of glass effect

    CGlassWindow() = default;
    ~CGlassWindow() = default;
    CGlassWindow(const CGlassWindow&) = delete;
    CGlassWindow& operator=(const CGlassWindow&) = delete;

    void registerConfig() {
        // Register config keys with Hyprlang::CConfigValue
        HyprlandAPI::addConfigValue(m_pluginHandle, "plugin:glasswindow:rules", Hyprlang::STRING{".*"});
        HyprlandAPI::addConfigValue(m_pluginHandle, "plugin:glasswindow:strength", Hyprlang::FLOAT{0.7f});
        HyprlandAPI::addConfigValue(m_pluginHandle, "plugin:glasswindow:chromatic_aberration", Hyprlang::FLOAT{0.0f});
        HyprlandAPI::addConfigValue(m_pluginHandle, "plugin:glasswindow:opacity", Hyprlang::FLOAT{0.9f});
    }

    void reloadConfig() {
        static auto* const PRULES = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(m_pluginHandle, "plugin:glasswindow:rules")->getDataStaticPtr();
        static auto* const PSTRENGTH = (Hyprlang::FLOAT const*)HyprlandAPI::getConfigValue(m_pluginHandle, "plugin:glasswindow:strength")->getDataStaticPtr();
        static auto* const PCHROMATIC = (Hyprlang::FLOAT const*)HyprlandAPI::getConfigValue(m_pluginHandle, "plugin:glasswindow:chromatic_aberration")->getDataStaticPtr();
        static auto* const POPACITY = (Hyprlang::FLOAT const*)HyprlandAPI::getConfigValue(m_pluginHandle, "plugin:glasswindow:opacity")->getDataStaticPtr();

        m_rulesRaw = *PRULES;
        m_strength = *PSTRENGTH;
        m_chromaticAberration = *PCHROMATIC;
        m_opacity = *POPACITY;

        parseRules(m_rulesRaw);
    }

    void parseRules(const std::string& rulesRaw) {
        m_rules.clear();

        // Assume rulesRaw is a semicolon-separated list of regex patterns
        size_t start = 0;
        while (true) {
            size_t end = rulesRaw.find(';', start);
            std::string rule = rulesRaw.substr(start, (end == std::string::npos ? rulesRaw.size() : end) - start);

            if (!rule.empty()) {
                try {
                    m_rules.emplace_back(rule, std::regex::ECMAScript | std::regex::icase);
                } catch (const std::regex_error& e) {
                    HyprlandAPI::addNotificationV2(m_pluginHandle, {
                        {"text", "glasswindow: Invalid regex: " + rule},
                        {"time", (uint64_t)5000},
                        {"color", CHyprColor(1.0, 0.0, 0.0, 1.0)}
                    });
                }
            }

            if (end == std::string::npos) break;
            start = end + 1;
        }
    }

    bool shouldApplyToWindow(const std::string& windowTitle) {
        for (const auto& rule : m_rules) {
            if (std::regex_search(windowTitle, rule)) {
                return true;
            }
        }
        return false;
    }

    void applyGlassEffect(void* window) {
        // TODO: Implement your shader or blur effect here, using m_strength, m_chromaticAberration, m_opacity
    }

    void onRenderWindow(void* thisptr, SCallbackInfo& info, std::any data) {
        // TODO: Extract window from data and apply effect
        // The actual data type depends on the event
    }
};

// Plugin entry point - must return PLUGIN_DESCRIPTION_INFO
APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;
    CGlassWindow::getInstance().init(handle);

    return PLUGIN_DESCRIPTION_INFO{
        "glasswindow",
        "Glass window effect plugin for Hyprland",
        "purplelines",
        "1.0.0"
    };
}

// Plugin exit point
APICALL EXPORT void PLUGIN_EXIT() {
    CGlassWindow::getInstance().cleanup();
}
