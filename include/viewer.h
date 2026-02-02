#ifndef SMSTRIKERS_VIEWER_H
#define SMSTRIKERS_VIEWER_H

#include <glad/gl.h>
#include <imgui.h>
#include <string>
#include <memory>
#include <array>
#include <vector>
#include "config.h"
#include "asset_tree.h"
#include "asset_tree_view.h"
#include "asset_loader.h"

struct GLFWwindow;

namespace SMStrikers {

struct TextureBundle;

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
    void renderFolderPicker();
    
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
    void refreshAssetTree();
    void handleAssetSelection(const AssetNode* node);
    void openFolderPicker();
    void clearLoadedTextures();
    void buildLoadedTextures(const TextureBundle& bundle);

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
    AssetTreeModel m_assetTreeModel;
    AssetTreeView m_assetTreeView;
    AssetLoaderRegistry m_assetLoaders;
    std::string m_selectedAssetPath;
    std::string m_lastLoadedPath;
    AssetLoadResult m_lastLoadResult;
    std::string m_lastLoaderName;
    bool m_hasLoadResult = false;

    struct LoadedTexture {
        uint32_t hash = 0;
        uint16_t width = 0;
        uint16_t height = 0;
        uint32_t format = 0;
        GLuint textureId = 0;
    };
    std::vector<LoadedTexture> m_loadedTextures;
    int m_selectedTextureIndex = 0;
    std::string m_loadedTexturePath;
    float m_thumbnailSize = 72.0f;
    float m_textureZoom = 1.0f;
    ImVec2 m_texturePan = ImVec2(0.0f, 0.0f);

    std::array<char, 512> m_assetsRootBuffer{};
    bool m_openFolderPicker = false;
    std::string m_folderPickerPath;
    std::string m_folderPickerSelected;
};

} // namespace SMStrikers

#endif // SMSTRIKERS_VIEWER_H
