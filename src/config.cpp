#include "config.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace SMStrikers {

std::string Config::getDefaultPath() {
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/.smstrikers-viewer.conf";
    }
    return ".smstrikers-viewer.conf";
}

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Config file not found, using defaults: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        // Parse key=value
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Parse values
        if (key == "invertPanX") {
            invertPanX = (value == "true" || value == "1");
        } else if (key == "invertPanY") {
            invertPanY = (value == "true" || value == "1");
        } else if (key == "defaultRenderMode") {
            defaultRenderMode = std::stoi(value);
        } else if (key == "showGizmo") {
            showGizmo = (value == "true" || value == "1");
        } else if (key == "showCameraInfo") {
            showCameraInfo = (value == "true" || value == "1");
        } else if (key == "showControls") {
            showControls = (value == "true" || value == "1");
        }
    }
    
    std::cout << "Loaded config from: " << filename << std::endl;
    return true;
}

bool Config::save(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save config to: " << filename << std::endl;
        return false;
    }
    
    file << "# Super Mario Strikers Viewer Configuration\n";
    file << "\n# Camera Settings\n";
    file << "invertPanX=" << (invertPanX ? "true" : "false") << "\n";
    file << "invertPanY=" << (invertPanY ? "true" : "false") << "\n";
    file << "\n# Render Settings\n";
    file << "defaultRenderMode=" << defaultRenderMode << "\n";
    file << "\n# UI Settings\n";
    file << "showGizmo=" << (showGizmo ? "true" : "false") << "\n";
    file << "showCameraInfo=" << (showCameraInfo ? "true" : "false") << "\n";
    file << "showControls=" << (showControls ? "true" : "false") << "\n";
    
    std::cout << "Saved config to: " << filename << std::endl;
    return true;
}

} // namespace SMStrikers
