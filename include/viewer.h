#ifndef SMSTRIKERS_VIEWER_H
#define SMSTRIKERS_VIEWER_H

#include <glad/gl.h>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include "config.h"

struct GLFWwindow;

namespace SMStrikers {

class Camera;
class Mesh;
class Shader;

/**
 * @brief Render mode for 3D viewport
 */
enum class RenderMode {
    Wireframe,
    Opaque,
    Shaded
};

/**
 * @brief Asset item in the tree view
 */
struct AssetItem {
    std::string name;
    std::string type;
    std::string path;  // File path for loadable assets
    std::vector<AssetItem> children;
    bool isSelected = false;
    
    // Mesh statistics (for mesh/model types)
    int vertexCount = 0;
    int triangleCount = 0;
    int faceCount = 0;
};

/**
 * @brief Main viewer application class
 * 
 * This class manages the OpenGL context, window, and rendering loop
 * for the Super Mario Strikers asset viewer.
 */
class Viewer {
public:
    Viewer();
    ~Viewer();

    /**
     * @brief Initialize the viewer
     * @param width Window width
     * @param height Window height
     * @param title Window title
     * @param noGui Run without GUI (direct rendering only)
     * @return true if initialization was successful
     */
    bool initialize(int width, int height, const std::string& title, bool noGui = false);
    
    /**
     * @brief Set which object should be rendered (for no-gui mode)
     * @param objectName Name of object to render
     */
    void setObjectToRender(const std::string& objectName);

    /**
     * @brief Run the main application loop
     * @return Exit code (0 for success)
     */
    int run();

    /**
     * @brief Clean up resources
     */
    void shutdown();

private:
    // Initialization
    bool initWindow();
    bool initOpenGL();
    bool initImGui();
    
    // Main loop
    void processInput();
    void update(float deltaTime);
    void render();
    
    // UI rendering
    void renderUI();
    void renderAssetTree();
    void renderViewport();
    void renderMenuBar();
    void renderConfigDialog();
    
    // Direct rendering (no GUI)
    void renderDirectMode();
    
    // 3D scene rendering
    void render3DScene(int width, int height);
    
    // Framebuffer management
    void createFramebuffer(int width, int height);
    void deleteFramebuffer();
    
    // Input handling
    void handleMouseInput();
    
    // Static callbacks
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // Asset management
    void loadDummyAssets(); // For testing
    bool loadAssetsFromJSON(const std::string& jsonPath);
    void parseAssetItem(const nlohmann::json& jsonItem, AssetItem& item);

    bool m_initialized;
    bool m_noGui;
    bool m_showConfigDialog;
    int m_windowWidth;
    int m_windowHeight;
    std::string m_windowTitle;
    
    Config m_config;
    
    GLFWwindow* m_window;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Mesh> m_dummyMesh;
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Shader> m_unlitShader;
    
    // Render settings
    RenderMode m_renderMode;
    
    // Framebuffer for viewport rendering
    GLuint m_framebuffer;
    GLuint m_framebufferTexture;
    GLuint m_depthRenderbuffer;
    int m_framebufferWidth;
    int m_framebufferHeight;
    
    // Mouse state
    double m_lastMouseX;
    double m_lastMouseY;
    bool m_isRotating;
    bool m_isPanning;
    bool m_isViewportHovered;
    
    // Assets
    std::vector<AssetItem> m_assetTree;
    AssetItem* m_selectedAsset;
};

} // namespace SMStrikers

#endif // SMSTRIKERS_VIEWER_H
