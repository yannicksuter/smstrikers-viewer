#ifndef SMSTRIKERS_CONFIG_H
#define SMSTRIKERS_CONFIG_H

#include <string>
#include <fstream>

namespace SMStrikers {

/**
 * @brief Configuration settings for the viewer
 */
struct Config {
    // Camera settings
    bool invertPanX = false;
    bool invertPanY = false;
    
    // Render settings
    int defaultRenderMode = 2; // 0=Wireframe, 1=Opaque, 2=Shaded
    
    // UI settings
    bool showGizmo = true;
    bool showCameraInfo = true;
    bool showControls = true;
    
    // Font settings
    int fontOversampleH = 3;
    int fontOversampleV = 2;
    bool fontPixelSnapH = false;
    
    /**
     * @brief Load config from file
     */
    bool load(const std::string& filename);
    
    /**
     * @brief Save config to file
     */
    bool save(const std::string& filename) const;
    
    /**
     * @brief Get default config file path
     */
    static std::string getDefaultPath();
};

} // namespace SMStrikers

#endif // SMSTRIKERS_CONFIG_H
