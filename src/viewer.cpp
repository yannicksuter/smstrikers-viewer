#include "viewer.h"
#include "camera.h"
#include "mesh.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <iostream>
#include <filesystem>
#include <cstdio>
#include <cfloat>
#include <cmath>
#include <functional>
#include <algorithm>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SMStrikers {

namespace {

const char* textureFormatLabel(uint32_t format) {
    switch (format) {
    case 0:
        return "RGB565";
    case 1:
        return "RGB5A3";
    case 2:
        return "CMPR";
    case 3:
        return "RGBA8";
    case 4:
        return "I8";
    case 5:
        return "I4";
    case 6:
        return "A8";
    case 7:
        return "IA8";
    case 8:
        return "CI8";
    default:
        return "Unknown";
    }
}

} // namespace

Viewer::Viewer()
    : m_initialized(false)
    , m_noGui(false)
    , m_showConfigDialog(false)
    , m_windowWidth(0)
    , m_windowHeight(0)
    , m_windowTitle("")
    , m_window(nullptr)
    , m_camera(nullptr)
    , m_renderMode(RenderMode::Shaded)
    , m_framebuffer(0)
    , m_framebufferTexture(0)
    , m_depthRenderbuffer(0)
    , m_framebufferWidth(0)
    , m_framebufferHeight(0)
    , m_lastMouseX(0.0)
    , m_lastMouseY(0.0)
    , m_isRotating(false)
    , m_isPanning(false)
    , m_isViewportHovered(false)
    , m_selectedAssetPath("")
    , m_lastLoadedPath("")
    , m_lastLoadResult()
    , m_lastLoaderName("")
    , m_hasLoadResult(false)
    , m_openFolderPicker(false)
    , m_folderPickerPath("")
    , m_folderPickerSelected("")
{
}

Viewer::~Viewer() {
    if (m_initialized) {
        shutdown();
    }
}

bool Viewer::initialize(int width, int height, const std::string& title, bool noGui) {
    std::cout << "Initializing Super Mario Strikers Viewer..." << std::endl;
    
    m_windowWidth = width;
    m_windowHeight = height;
    m_windowTitle = title;
    m_noGui = noGui;
    
    if (!initWindow()) {
        std::cerr << "Failed to initialize window!" << std::endl;
        return false;
    }
    
    if (!initOpenGL()) {
        std::cerr << "Failed to initialize OpenGL!" << std::endl;
        return false;
    }
    
    if (!m_noGui) {
        if (!initImGui()) {
            std::cerr << "Failed to initialize ImGui!" << std::endl;
            return false;
        }
    }
    
    // Initialize camera
    m_camera = std::make_unique<Camera>();
    
    // Load config
    m_config.load(Config::getDefaultPath());
    m_renderMode = static_cast<RenderMode>(m_config.defaultRenderMode);
    std::snprintf(m_assetsRootBuffer.data(), m_assetsRootBuffer.size(), "%s", m_config.assetsRoot.c_str());
    
    // Create dummy mesh and shaders
    m_dummyMesh.reset(Mesh::createCube(2.0f));
    
    m_shader = std::make_unique<Shader>();
    if (!m_shader->createBasicShader()) {
        std::cerr << "Failed to create lit shader!" << std::endl;
        return false;
    }
    
    m_unlitShader = std::make_unique<Shader>();
    if (!m_unlitShader->createUnlitShader()) {
        std::cerr << "Failed to create unlit shader!" << std::endl;
        return false;
    }
    
    // Load assets from filesystem
    refreshAssetTree();
    
    m_initialized = true;
    std::cout << "Initialization complete!" << std::endl;
    
    return m_initialized;
}

bool Viewer::initWindow() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, 
                                m_windowTitle.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync
    
    // Set user pointer for callbacks
    glfwSetWindowUserPointer(m_window, this);
    
    // Set scroll callback for zoom
    glfwSetScrollCallback(m_window, scrollCallback);
    
    return true;
}

bool Viewer::initOpenGL() {
    // Initialize GLAD
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return false;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // Configure OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    return true;
}

bool Viewer::initImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    // Enable keyboard navigation and docking
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    // Setup style
    ImGui::StyleColorsDark();
    
    // Configure font rendering for better quality
    ImFontConfig fontConfig;
    fontConfig.OversampleH = m_config.fontOversampleH;  // Horizontal oversampling
    fontConfig.OversampleV = m_config.fontOversampleV;  // Vertical oversampling
    fontConfig.PixelSnapH = m_config.fontPixelSnapH;  // Subpixel positioning
    
    // Load default font with antialiasing
    io.Fonts->AddFontDefault(&fontConfig);
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    return true;
}


int Viewer::run() {
    if (!m_initialized) {
        std::cerr << "Error: Viewer not initialized!" << std::endl;
        return -1;
    }
    
    std::cout << "Starting main loop..." << std::endl;
    
    float lastFrame = 0.0f;
    
    // Main loop
    while (!glfwWindowShouldClose(m_window)) {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Process input
        processInput();
        
        // Update
        update(deltaTime);
        
        // Render
        render();
        
        // Swap buffers and poll events
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    
    std::cout << "Main loop ended." << std::endl;
    return 0;
}

void Viewer::processInput() {
    // ESC to close
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
    
    // Handle mouse input for camera
    handleMouseInput();
}

void Viewer::handleMouseInput() {
    double mouseX, mouseY;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);
    
    double deltaX = mouseX - m_lastMouseX;
    double deltaY = mouseY - m_lastMouseY;
    
    // In no-gui mode, always allow mouse input
    bool allowInput = m_noGui || m_isViewportHovered;
    
    // Right mouse button: rotate (only when viewport is hovered or no-gui mode)
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // Start rotating on first press if over viewport, then continue regardless
        if (!m_isRotating) {
            m_isRotating = allowInput;
        }
        if (m_isRotating && (deltaX != 0 || deltaY != 0)) {
            m_camera->rotate(static_cast<float>(deltaX), static_cast<float>(-deltaY));
        }
    } else {
        m_isRotating = false;
    }
    
    // Middle mouse button: pan
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        if (!m_isPanning) {
            m_isPanning = allowInput;
        }
        if (m_isPanning && (deltaX != 0 || deltaY != 0)) {
            float panX = static_cast<float>(m_config.invertPanX ? deltaX : -deltaX);
            float panY = static_cast<float>(m_config.invertPanY ? -deltaY : deltaY);
            m_camera->pan(panX, panY);
        }
    } else {
        m_isPanning = false;
    }
    
    // Keyboard zoom (kept as alternative)
    if (glfwGetKey(m_window, GLFW_KEY_EQUAL) == GLFW_PRESS || 
        glfwGetKey(m_window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        m_camera->zoom(0.1f);
    }
    if (glfwGetKey(m_window, GLFW_KEY_MINUS) == GLFW_PRESS || 
        glfwGetKey(m_window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        m_camera->zoom(-0.1f);
    }
    
    // Reset camera
    if (glfwGetKey(m_window, GLFW_KEY_HOME) == GLFW_PRESS) {
        m_camera->reset();
    }
    
    m_lastMouseX = mouseX;
    m_lastMouseY = mouseY;
}

void Viewer::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)xoffset; // Unused
    Viewer* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
    if (viewer && viewer->m_camera) {
        // Zoom with mouse wheel
        viewer->m_camera->zoom(static_cast<float>(yoffset));
    }
}

void Viewer::update(float deltaTime) {
    // Future: Update animations, etc.
    (void)deltaTime; // Unused for now
}

void Viewer::render() {
    // Get framebuffer size
    int displayW, displayH;
    glfwGetFramebufferSize(m_window, &displayW, &displayH);
    
    // Clear
    glViewport(0, 0, displayW, displayH);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (m_noGui) {
        // Direct 3D rendering without UI
        renderDirectMode();
    } else {
        // Render UI
        renderUI();
    }
}

void Viewer::renderUI() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    
    // Main menu bar
    renderMenuBar();
    
    // Setup dockspace over the viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    
    // Create dockspace
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    
    ImGui::End();
    
    // Render Assets first so it can dock left, then Viewport fills remaining space
    renderAssetTree();
    renderViewport();
    
    // Config dialog
    if (m_showConfigDialog) {
        renderConfigDialog();
    }
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void Viewer::setObjectToRender(const std::string& objectName) {
    std::cout << "Setting object to render: " << objectName << std::endl;
    if (objectName.empty()) {
        return;
    }

    const AssetNode* nodeByPath = m_assetTreeModel.findByPath(objectName);
    if (nodeByPath) {
        m_selectedAssetPath = nodeByPath->relativePath;
        handleAssetSelection(nodeByPath);
        return;
    }

    std::function<const AssetNode*(const std::vector<AssetNode>&)> findByName;
    findByName = [&](const std::vector<AssetNode>& nodes) -> const AssetNode* {
        for (const auto& node : nodes) {
            if (node.name == objectName) {
                return &node;
            }
            if (!node.children.empty()) {
                const AssetNode* found = findByName(node.children);
                if (found) {
                    return found;
                }
            }
        }
        return nullptr;
    };

    const AssetNode* nodeByName = findByName(m_assetTreeModel.roots());
    if (nodeByName) {
        m_selectedAssetPath = nodeByName->relativePath;
        handleAssetSelection(nodeByName);
    }
}

void Viewer::renderDirectMode() {
    // Ensure proper OpenGL state for 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    
    // Set polygon mode based on render mode
    if (m_renderMode == RenderMode::Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Get view and projection matrices
    glm::mat4 view = m_camera->getViewMatrix();
    float aspect = static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight);
    glm::mat4 projection = m_camera->getProjectionMatrix(aspect);
    
    // Select shader based on render mode
    Shader* activeShader = (m_renderMode == RenderMode::Shaded) ? m_shader.get() : m_unlitShader.get();
    activeShader->use();
    
    glm::mat4 model = glm::mat4(1.0f);
    activeShader->setMat4("model", model);
    activeShader->setMat4("view", view);
    activeShader->setMat4("projection", projection);
    
    // Set lighting uniforms only for shaded mode
    if (m_renderMode == RenderMode::Shaded) {
        glm::vec3 lightPos = m_camera->getPosition();
        activeShader->setVec3("lightPos", lightPos);
        activeShader->setVec3("viewPos", m_camera->getPosition());
    }
    
    m_dummyMesh->render();
    
    // Reset polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Viewer::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Asset Folder...")) {
                // TODO: File browser
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "ESC")) {
                glfwSetWindowShouldClose(m_window, true);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Reset Camera", "Home")) {
                m_camera->reset();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Settings...")) {
                m_showConfigDialog = true;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // TODO: About dialog
            }
            ImGui::EndMenu();
        }
        
        // Show FPS on right side
        float fps = ImGui::GetIO().Framerate;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 120);
        ImGui::Text("%.1f FPS", fps);
        
        ImGui::EndMainMenuBar();
    }
}

void Viewer::renderAssetTree() {
    // Initial size
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowSize(ImVec2(300, viewport->WorkSize.y), ImGuiCond_FirstUseEver);
    
    // Dock to left side on first use
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Assets");

    ImGui::Text("Root: %s", m_config.assetsRoot.c_str());
    if (!m_assetTreeModel.hasRoot() || m_assetTreeModel.roots().empty()) {
        ImGui::TextDisabled("No assets found");
    }

    std::string previousSelection = m_selectedAssetPath;

    // Render root items in a child window (top section)
    ImGui::BeginChild("AssetTreeView", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 8), false);
    if (!m_assetTreeModel.roots().empty()) {
        m_assetTreeView.renderTree(m_assetTreeModel.roots(), m_selectedAssetPath);
    }
    ImGui::EndChild();

    if (previousSelection != m_selectedAssetPath) {
        const AssetNode* selectedNode = m_assetTreeModel.findByPath(m_selectedAssetPath);
        handleAssetSelection(selectedNode);
    }
    
    // Properties section (bottom section)
    ImGui::Separator();
    ImGui::Text("Properties");
    ImGui::Separator();
    
    const AssetNode* selectedNode = m_assetTreeModel.findByPath(m_selectedAssetPath);
    if (selectedNode) {
        ImGui::Text("Name: %s", selectedNode->name.c_str());
        ImGui::Text("Type: %s", assetKindLabel(selectedNode->kind));
        ImGui::Text("Path: %s", selectedNode->relativePath.c_str());

        ImGui::Separator();
        ImGui::Text("Loadable: %s", isLoadable(selectedNode->kind) ? "Yes" : "No");
        if (selectedNode->kind == AssetKind::TextureBundle) {
            ImGui::Text("Package: Texture Bundle (.glt)");
        } else if (selectedNode->kind == AssetKind::ModelBundle) {
            ImGui::Text("Package: Model Bundle (.glg)");
        }

        if (m_hasLoadResult && m_lastLoadedPath == selectedNode->relativePath) {
            ImGui::Separator();
            ImGui::Text("Loader: %s", m_lastLoaderName.empty() ? "Unknown" : m_lastLoaderName.c_str());
            ImGui::Text("Status: %s", m_lastLoadResult.success ? "Loaded" : "Failed");
            if (!m_lastLoadResult.message.empty()) {
                ImGui::Text("Message: %s", m_lastLoadResult.message.c_str());
            }
            if (m_lastLoadResult.fileSize > 0) {
                ImGui::Text("File Size: %llu bytes", static_cast<unsigned long long>(m_lastLoadResult.fileSize));
            }
            if (selectedNode->kind == AssetKind::TextureBundle && m_lastLoadedPath == selectedNode->relativePath) {
                ImGui::Separator();
                if (m_loadedTextures.empty()) {
                    ImGui::TextDisabled("No decoded textures");
                } else {
                    ImGui::Text("Textures: %zu", m_loadedTextures.size());
                    ImVec2 listSize = ImVec2(-FLT_MIN, 160.0f);
                    if (ImGui::BeginListBox("##glt_textures", listSize)) {
                        for (size_t i = 0; i < m_loadedTextures.size(); ++i) {
                            const auto& tex = m_loadedTextures[i];
                            bool isSelected = static_cast<int>(i) == m_selectedTextureIndex;
                            char label[128];
                            std::snprintf(label, sizeof(label), "0x%08X  %ux%u", tex.hash, tex.width, tex.height);
                            if (ImGui::Selectable(label, isSelected)) {
                                m_selectedTextureIndex = static_cast<int>(i);
                                m_textureZoom = 1.0f;
                                m_texturePan = ImVec2(0.0f, 0.0f);
                            }
                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndListBox();
                    }

                    ImGui::Separator();
                    ImGui::Text("Thumbnails");
                    ImGui::SliderFloat("##thumb_size", &m_thumbnailSize, 32.0f, 160.0f, "%.0f px");
                    ImVec2 childSize = ImVec2(0.0f, 220.0f);
                    ImGui::BeginChild("TextureThumbs", childSize, true, ImGuiWindowFlags_HorizontalScrollbar);
                    float panelWidth = ImGui::GetContentRegionAvail().x;
                    float padding = ImGui::GetStyle().ItemSpacing.x;
                    float cellSize = m_thumbnailSize + padding;
                    int columns = static_cast<int>(panelWidth / cellSize);
                    if (columns < 1) {
                        columns = 1;
                    }
                    int columnIndex = 0;
                    for (size_t i = 0; i < m_loadedTextures.size(); ++i) {
                        const auto& tex = m_loadedTextures[i];
                        if (tex.textureId == 0) {
                            continue;
                        }
                        ImGui::PushID(static_cast<int>(i));
                        ImVec2 imageSize(m_thumbnailSize, m_thumbnailSize);
                        if (ImGui::ImageButton("##thumb", (void*)(intptr_t)tex.textureId, imageSize, ImVec2(0, 0), ImVec2(1, 1))) {
                            m_selectedTextureIndex = static_cast<int>(i);
                            m_textureZoom = 1.0f;
                            m_texturePan = ImVec2(0.0f, 0.0f);
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("0x%08X", tex.hash);
                            ImGui::Text("%ux%u", tex.width, tex.height);
                            ImGui::Text("%s", textureFormatLabel(tex.format));
                            ImGui::EndTooltip();
                        }
                        if (static_cast<int>(i) == m_selectedTextureIndex) {
                            ImDrawList* drawList = ImGui::GetWindowDrawList();
                            ImVec2 min = ImGui::GetItemRectMin();
                            ImVec2 max = ImGui::GetItemRectMax();
                            drawList->AddRect(min, max, IM_COL32(255, 200, 64, 255), 0.0f, 0, 2.0f);
                        }
                        ImGui::PopID();
                        columnIndex++;
                        if (columnIndex < columns) {
                            ImGui::SameLine();
                        } else {
                            columnIndex = 0;
                        }
                    }
                    ImGui::EndChild();
                }
            }
        }
    } else {
        ImGui::TextDisabled("No asset selected");
    }
    
    ImGui::End();
}

void Viewer::refreshAssetTree() {
    m_assetTreeModel.loadFromFilesystem(m_config.assetsRoot);

    if (!m_selectedAssetPath.empty()) {
        const AssetNode* node = m_assetTreeModel.findByPath(m_selectedAssetPath);
        if (!node) {
            m_selectedAssetPath.clear();
        }
    }

    if (m_selectedAssetPath.empty()) {
        m_hasLoadResult = false;
        m_lastLoadedPath.clear();
        m_lastLoaderName.clear();
        clearLoadedTextures();
    }
}

void Viewer::handleAssetSelection(const AssetNode* node) {
    m_hasLoadResult = false;
    m_lastLoadedPath.clear();
    m_lastLoaderName.clear();
    clearLoadedTextures();

    if (!node) {
        return;
    }

    if (!isLoadable(node->kind)) {
        return;
    }

    std::filesystem::path fullPath = std::filesystem::path(m_assetTreeModel.rootPath()) / node->relativePath;
    const IAssetLoader* loader = m_assetLoaders.getLoaderForExtension(fullPath.extension().string());
    if (!loader) {
        m_lastLoadResult = {false, "No loader registered", 0};
        m_hasLoadResult = true;
        m_lastLoadedPath = node->relativePath;
        return;
    }

    m_lastLoadResult = loader->load(fullPath);
    m_lastLoaderName = loader->name();
    m_lastLoadedPath = node->relativePath;
    m_hasLoadResult = true;

    if (node->kind == AssetKind::TextureBundle && m_lastLoadResult.success && m_lastLoadResult.textureBundle) {
        buildLoadedTextures(*m_lastLoadResult.textureBundle);
    }
}

void Viewer::renderViewport() {
    // Viewport can't be closed but has title bar for proper decoration
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    
    // Dock to dockspace on first use (will take up remaining space)
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport", nullptr, window_flags);
    
    // Check if mouse is hovering over this window's content area
    m_isViewportHovered = ImGui::IsWindowHovered();
    
    // Get viewport size
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        const AssetNode* selectedNode = m_assetTreeModel.findByPath(m_selectedAssetPath);
        bool isTexturePreview = selectedNode && selectedNode->kind == AssetKind::TextureBundle &&
                                !m_loadedTextures.empty() && m_lastLoadedPath == selectedNode->relativePath;
        bool canRender = selectedNode && isLoadable(selectedNode->kind) && !isTexturePreview;

        if (isTexturePreview) {
            const auto& texture = m_loadedTextures[std::clamp(m_selectedTextureIndex, 0, static_cast<int>(m_loadedTextures.size() - 1))];
            if (m_isViewportHovered) {
                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f) {
                    float zoomFactor = std::pow(1.1f, wheel);
                    m_textureZoom = std::clamp(m_textureZoom * zoomFactor, 0.1f, 32.0f);
                }
                if (ImGui::IsKeyPressed(ImGuiKey_R)) {
                    m_textureZoom = 1.0f;
                    m_texturePan = ImVec2(0.0f, 0.0f);
                }
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    m_texturePan.x += delta.x;
                    m_texturePan.y += delta.y;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                }
            }
            float scaleX = viewportSize.x / static_cast<float>(texture.width);
            float scaleY = viewportSize.y / static_cast<float>(texture.height);
            float baseScale = std::min(1.0f, std::min(scaleX, scaleY));
            float scale = baseScale * m_textureZoom;
            ImVec2 imageSize(texture.width * scale, texture.height * scale);
            ImVec2 imagePos((viewportSize.x - imageSize.x) * 0.5f + m_texturePan.x,
                            (viewportSize.y - imageSize.y) * 0.5f + m_texturePan.y);
            ImGui::SetCursorPos(imagePos);
            ImGui::Image((void*)(intptr_t)texture.textureId, imageSize, ImVec2(0, 0), ImVec2(1, 1));

            ImGui::SetCursorPos(ImVec2(10.0f, 10.0f));
            ImGui::Text("Texture 0x%08X", texture.hash);
            ImGui::Text("Size: %ux%u", texture.width, texture.height);
            ImGui::Text("Format: %s", textureFormatLabel(texture.format));
        } else if (canRender) {
            // Recreate framebuffer if size changed
            if (m_framebuffer == 0 || 
                m_framebufferWidth != static_cast<int>(viewportSize.x) || 
                m_framebufferHeight != static_cast<int>(viewportSize.y)) {
                createFramebuffer(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
            }
            
            // Render to framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
            glViewport(0, 0, m_framebufferWidth, m_framebufferHeight);
            
            render3DScene(m_framebufferWidth, m_framebufferHeight);
            
            // Unbind framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            // Display the framebuffer texture in ImGui
            ImGui::Image((void*)(intptr_t)m_framebufferTexture, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
            
            // Draw gizmo overlay if enabled
            if (m_config.showGizmo) {
                ImVec2 p = ImGui::GetItemRectMin();
                ImVec2 gizmoPos(p.x + viewportSize.x - 100, p.y + 10);
                float gizmoSize = 80.0f;
                
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(p.x, p.y, viewportSize.x, viewportSize.y);
                
                glm::mat4 view = m_camera->getViewMatrix();
                ImGuizmo::ViewManipulate(glm::value_ptr(view), m_camera->getDistance(), 
                                        ImVec2(gizmoPos.x, gizmoPos.y), 
                                        ImVec2(gizmoSize, gizmoSize), 0x10101010);
            }
        } else {
            // Draw placeholder text when nothing is selected
            ImVec2 textSize = ImGui::CalcTextSize("Select an asset to view");
            ImVec2 textPos((viewportSize.x - textSize.x) * 0.5f, (viewportSize.y - textSize.y) * 0.5f);
            ImGui::SetCursorPos(textPos);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Select an asset to view");
        }
        
        // Camera info overlay (only when object selected and enabled)
        if (canRender && m_config.showCameraInfo) {
            ImGui::SetCursorPos(ImVec2(10, 35));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Fully transparent
            ImGui::BeginChild("CamInfo", ImVec2(250, 50), false, ImGuiWindowFlags_NoScrollbar);
            glm::vec3 camPos = m_camera->getPosition();
            ImGui::Text("Camera: %.1f, %.1f, %.1f", camPos.x, camPos.y, camPos.z);
            ImGui::Text("Distance: %.1f", m_camera->getDistance());
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
            
        // Controls hint (only when object selected and enabled)
        if (canRender && m_config.showControls) {
            ImGui::SetCursorPos(ImVec2(10, viewportSize.y - 80));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Fully transparent
            ImGui::BeginChild("Controls", ImVec2(250, 70), false, ImGuiWindowFlags_NoScrollbar);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Right Mouse: Rotate");
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Middle Mouse: Pan");
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Mouse Wheel: Zoom");
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Home: Reset Camera");
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
}

void Viewer::shutdown() {
    std::cout << "Shutting down viewer..." << std::endl;
    
    // Cleanup framebuffer
    deleteFramebuffer();
    clearLoadedTextures();
    
    // Cleanup ImGui only if it was initialized
    if (!m_noGui) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    
    // Cleanup GLFW
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
    
    m_initialized = false;
}

void Viewer::createFramebuffer(int width, int height) {
    // Delete existing framebuffer if it exists
    if (m_framebuffer != 0) {
        deleteFramebuffer();
    }
    
    m_framebufferWidth = width;
    m_framebufferHeight = height;
    
    // Create framebuffer
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    
    // Create color texture
    glGenTextures(1, &m_framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, m_framebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebufferTexture, 0);
    
    // Create depth renderbuffer
    glGenRenderbuffers(1, &m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewer::deleteFramebuffer() {
    if (m_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
    if (m_framebufferTexture != 0) {
        glDeleteTextures(1, &m_framebufferTexture);
        m_framebufferTexture = 0;
    }
    if (m_depthRenderbuffer != 0) {
        glDeleteRenderbuffers(1, &m_depthRenderbuffer);
        m_depthRenderbuffer = 0;
    }
}

void Viewer::clearLoadedTextures() {
    for (auto& texture : m_loadedTextures) {
        if (texture.textureId != 0) {
            glDeleteTextures(1, &texture.textureId);
        }
    }
    m_loadedTextures.clear();
    m_selectedTextureIndex = 0;
    m_loadedTexturePath.clear();
    m_textureZoom = 1.0f;
    m_texturePan = ImVec2(0.0f, 0.0f);
}

void Viewer::buildLoadedTextures(const TextureBundle& bundle) {
    clearLoadedTextures();
    if (bundle.textures.empty()) {
        return;
    }

    m_loadedTextures.reserve(bundle.textures.size());
    GLint previousAlignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (const auto& image : bundle.textures) {
        if (image.rgba.empty() || image.width == 0 || image.height == 0) {
            continue;
        }
        GLuint textureId = 0;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        LoadedTexture entry;
        entry.hash = image.hash;
        entry.width = image.width;
        entry.height = image.height;
        entry.format = image.format;
        entry.textureId = textureId;
        m_loadedTextures.push_back(entry);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
    m_loadedTexturePath = m_lastLoadedPath;
    m_selectedTextureIndex = 0;
    m_textureZoom = 1.0f;
    m_texturePan = ImVec2(0.0f, 0.0f);
}

void Viewer::renderConfigDialog() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", &m_showConfigDialog)) {
        bool configChanged = false;
        
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Camera Settings");
        ImGui::Separator();
        if (ImGui::Checkbox("Invert Pan X", &m_config.invertPanX)) configChanged = true;
        if (ImGui::Checkbox("Invert Pan Y", &m_config.invertPanY)) configChanged = true;
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Render Settings");
        ImGui::Separator();
        const char* modes[] = { "Wireframe", "Opaque", "Shaded" };
        if (ImGui::Combo("Render Mode", &m_config.defaultRenderMode, modes, 3)) {
            m_renderMode = static_cast<RenderMode>(m_config.defaultRenderMode);
            configChanged = true;
        }
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "UI Settings");
        ImGui::Separator();
        if (ImGui::Checkbox("Show Gizmo", &m_config.showGizmo)) configChanged = true;
        if (ImGui::Checkbox("Show Camera Info", &m_config.showCameraInfo)) configChanged = true;
        if (ImGui::Checkbox("Show Controls", &m_config.showControls)) configChanged = true;
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Font Settings");
        ImGui::Separator();
        bool fontChanged = false;
        if (ImGui::SliderInt("Horizontal Oversampling", &m_config.fontOversampleH, 1, 5)) {
            configChanged = true;
            fontChanged = true;
        }
        if (ImGui::SliderInt("Vertical Oversampling", &m_config.fontOversampleV, 1, 5)) {
            configChanged = true;
            fontChanged = true;
        }
        if (ImGui::Checkbox("Pixel Snap (less smooth)", &m_config.fontPixelSnapH)) {
            configChanged = true;
            fontChanged = true;
        }
        if (fontChanged) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Restart required for font changes");
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Asset Settings");
        ImGui::Separator();
        ImGui::InputText("Assets Root", m_assetsRootBuffer.data(), m_assetsRootBuffer.size());
        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
            openFolderPicker();
        }
        if (ImGui::Button("Apply & Rescan")) {
            std::string newRoot = std::string(m_assetsRootBuffer.data());
            if (newRoot != m_config.assetsRoot) {
                m_config.assetsRoot = newRoot;
                configChanged = true;
            }
            refreshAssetTree();
        }
        ImGui::SameLine();
        if (ImGui::Button("Rescan")) {
            refreshAssetTree();
        }
        const AssetTreeStats& stats = m_assetTreeModel.stats();
        ImGui::Text("Items: %zu (Folders: %zu, Files: %zu, Loadable: %zu)",
                    stats.nodeCount, stats.folderCount, stats.fileCount, stats.loadableCount);
        
        // Auto-save when config changes
        if (configChanged) {
            m_config.save(Config::getDefaultPath());
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
                          "Config file: %s", Config::getDefaultPath().c_str());
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
                          "Settings are saved automatically");
    }
    ImGui::End();

    renderFolderPicker();
}

void Viewer::openFolderPicker() {
    std::filesystem::path startPath = m_config.assetsRoot;
    if (startPath.empty() || !std::filesystem::exists(startPath)) {
        startPath = std::filesystem::current_path();
    }
    if (!std::filesystem::is_directory(startPath)) {
        startPath = startPath.parent_path();
    }
    m_folderPickerPath = startPath.string();
    m_folderPickerSelected.clear();
    m_openFolderPicker = true;
}

void Viewer::renderFolderPicker() {
    if (m_openFolderPicker) {
        ImGui::OpenPopup("Select Asset Root");
        m_openFolderPicker = false;
    }

    if (ImGui::BeginPopupModal("Select Asset Root", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::filesystem::path currentPath = m_folderPickerPath;
        if (currentPath.empty()) {
            currentPath = std::filesystem::current_path();
            m_folderPickerPath = currentPath.string();
        }

        ImGui::Text("Current: %s", currentPath.string().c_str());

        if (ImGui::Button("Up") && currentPath.has_parent_path()) {
            currentPath = currentPath.parent_path();
            m_folderPickerPath = currentPath.string();
            m_folderPickerSelected.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            m_folderPickerSelected.clear();
        }

        ImGui::Separator();

        ImGui::BeginChild("FolderPickerList", ImVec2(520, 300), true);
        std::vector<std::filesystem::path> directories;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
                if (entry.is_directory()) {
                    directories.push_back(entry.path());
                }
            }
        } catch (const std::exception&) {
        }

        std::sort(directories.begin(), directories.end(), [](const auto& a, const auto& b) {
            return a.filename().string() < b.filename().string();
        });

        if (directories.empty()) {
            ImGui::TextDisabled("No subfolders");
        } else {
            for (const auto& dir : directories) {
                const std::string name = dir.filename().string();
                const std::string fullPath = dir.string();
                bool selected = (m_folderPickerSelected == fullPath);
                if (ImGui::Selectable(name.c_str(), selected)) {
                    m_folderPickerSelected = fullPath;
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    m_folderPickerPath = fullPath;
                    m_folderPickerSelected.clear();
                }
            }
        }
        ImGui::EndChild();

        ImGui::Separator();

        std::filesystem::path selectedPath = currentPath;
        if (!m_folderPickerSelected.empty()) {
            selectedPath = m_folderPickerSelected;
        }

        if (ImGui::Button("Select")) {
            std::string chosen = selectedPath.string();
            std::snprintf(m_assetsRootBuffer.data(), m_assetsRootBuffer.size(), "%s", chosen.c_str());
            if (chosen != m_config.assetsRoot) {
                m_config.assetsRoot = chosen;
                m_config.save(Config::getDefaultPath());
            }
            refreshAssetTree();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Viewer::render3DScene(int width, int height) {
    // Ensure proper OpenGL state for 3D rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    
    // Clear
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set polygon mode based on render mode
    if (m_renderMode == RenderMode::Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Get view and projection matrices
    glm::mat4 view = m_camera->getViewMatrix();
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    glm::mat4 projection = m_camera->getProjectionMatrix(aspect);
    
    // Select shader based on render mode
    Shader* activeShader = (m_renderMode == RenderMode::Shaded) ? m_shader.get() : m_unlitShader.get();
    activeShader->use();
    
    glm::mat4 model = glm::mat4(1.0f);
    activeShader->setMat4("model", model);
    activeShader->setMat4("view", view);
    activeShader->setMat4("projection", projection);
    
    // Set lighting uniforms only for shaded mode
    if (m_renderMode == RenderMode::Shaded) {
        glm::vec3 lightPos = m_camera->getPosition();
        activeShader->setVec3("lightPos", lightPos);
        activeShader->setVec3("viewPos", m_camera->getPosition());
    }
    
    m_dummyMesh->render();
    
    // Reset polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

} // namespace SMStrikers
