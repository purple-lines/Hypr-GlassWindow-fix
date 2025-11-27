#include <hyprland/src/plugins/PluginAPI.hpp>
#include <regex>
#include <vector>
#include <string>
#include <optional>

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

        // Register render hook
        HyprlandAPI::registerCallback(m_pluginHandle, "renderWindow", 
            [this](void* data) { this->onRenderWindow(data); });

        // Register config reload hook (optional)
        HyprlandAPI::registerCallback(m_pluginHandle, "configReload", 
            [this](void*) { this->reloadConfig(); });
    }

    // Called once on plugin exit
    void cleanup() {
        // Unregister all hooks
        HyprlandAPI::unregisterCallback(m_pluginHandle, "renderWindow");
        HyprlandAPI::unregisterCallback(m_pluginHandle, "configReload");

        // TODO: free shaders/resources here
    }

  private:
    HANDLE m_pluginHandle = nullptr;

    // Config keys (strings, floats, ints)
    std::string m_rulesRaw;             // Raw regex/rule strings from config
    std::vector<std::regex> m_rules;   // Parsed regex rules

    float m_strength = 0.7f;            // Glass effect strength (blur etc)
    float m_chromaticAberration = 0.f; // Chromatic aberration strength
    float m_opacity = 0.9f;             // Opacity of glass effect

    // TODO: shader program handles

    CGlassWindow() = default;
    ~CGlassWindow() = default;
    CGlassWindow(const CGlassWindow&) = delete;
    CGlassWindow& operator=(const CGlassWindow&) = delete;

    void registerConfig() {
        // Register your config keys here with default values
        HyprlandAPI::addConfigValue(m_pluginHandle, "plugin:glasswindow:rules", ".*");
        HyprlandAPI::addConfigValue(m_pluginHandle, "plugin:glasswindow:strength", "0.7");
        HyprlandAPI::addConfigValue(m_pluginHandle, "plugin:glasswindow:chromatic_aberration", "0.0");
        HyprlandAPI::addConfigValue(m_pluginHandle, "plugin:glasswindow:opacity", "0.9");
    }

    void reloadConfig() {
        m_rulesRaw = HyprlandAPI::getConfigValue(m_pluginHandle, "plugin:glasswindow:rules");
        m_strength = std::stof(HyprlandAPI::getConfigValue(m_pluginHandle, "plugin:glasswindow:strength"));
        m_chromaticAberration = std::stof(HyprlandAPI::getConfigValue(m_pluginHandle, "plugin:glasswindow:chromatic_aberration"));
        m_opacity = std::stof(HyprlandAPI::getConfigValue(m_pluginHandle, "plugin:glasswindow:opacity"));

        parseRules(m_rulesRaw);

        // TODO: update shader uniforms here
    }

    void parseRules(const std::string& rulesRaw) {
        m_rules.clear();

        // Assume rulesRaw is a semicolon-separated list of regex patterns
        size_t start = 0;
        while (true) {
            size_t end = rulesRaw.find(';', start);
            std::string rule = rulesRaw.substr(start, (end == std::string::npos ? rulesRaw.size() : end) - start);

            try {
                m_rules.emplace_back(rule, std::regex::ECMAScript | std::regex::icase);
            } catch (const std::regex_error& e) {
                HyprlandAPI::addNotification(m_pluginHandle, "glasswindow", 
                    "Invalid regex in rules: " + rule, "error", 5000);
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

        // Placeholder:
        // HyprlandAPI::drawCustomEffect(window, m_strength, m_chromaticAberration, m_opacity);
    }

    void onRenderWindow(void* data) {
        // `data` usually points to a structure describing the window being rendered, e.g. CWindow*

        // Extract window title (pseudo-code)
        std::string windowTitle = getWindowTitleFromData(data);

        if (!shouldApplyToWindow(windowTitle))
            return;

        applyGlassEffect(data);
    }

    // Dummy placeholder for getting window title from `data`
    std::string getWindowTitleFromData(void* data) {
        // TODO: Use actual Hyprland API to get window title string
        return "ExampleWindowTitle";
    }
};

extern "C" {

// Plugin entry point called by Hyprland when plugin loads
void PLUGIN_INIT(HANDLE handle) {
    CGlassWindow::getInstance().init(handle);
}

// Plugin exit point called by Hyprland when plugin unloads
void PLUGIN_EXIT() {
    CGlassWindow::getInstance().cleanup();
}

}
