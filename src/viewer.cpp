#include "viewer.h"
#include "camera.h"
#include "mesh.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using json = nlohmann::json;

namespace SMStrikers {

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
    , m_selectedAsset(nullptr)
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
    
    // Load dummy assets for testing
    loadDummyAssets();
    
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
    
    // Enable keyboard navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    return true;
}

void Viewer::loadDummyAssets() {
    // Try to load from JSON first
    std::string jsonPath = "assets/assets.json";
    if (loadAssetsFromJSON(jsonPath)) {
        std::cout << "Loaded assets from: " << jsonPath << std::endl;
        return;
    }
    
    std::cout << "Could not load " << jsonPath << ", using dummy assets" << std::endl;
    
    // Fallback: Create dummy asset tree for testing
    AssetItem characters;
    characters.name = "Characters";
    characters.type = "folder";
    
    AssetItem mario;
    mario.name = "Mario";
    mario.type = "model";
    mario.children.push_back({"Body", "mesh", "", {}});
    mario.children.push_back({"Head", "mesh", "", {}});
    mario.children.push_back({"Skeleton", "bones", "", {}});
    characters.children.push_back(mario);
    
    AssetItem luigi;
    luigi.name = "Luigi";
    luigi.type = "model";
    luigi.children.push_back({"Body", "mesh", "", {}});
    luigi.children.push_back({"Head", "mesh", "", {}});
    characters.children.push_back(luigi);
    
    m_assetTree.push_back(characters);
    
    AssetItem environments;
    environments.name = "Environments";
    environments.type = "folder";
    environments.children.push_back({"Stadium", "model", "", {}});
    environments.children.push_back({"Field", "model", "", {}});
    
    m_assetTree.push_back(environments);
}

bool Viewer::loadAssetsFromJSON(const std::string& jsonPath) {
    try {
        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            return false;
        }
        
        json data = json::parse(file);
        
        // Check for version
        if (data.contains("version")) {
            std::cout << "Asset manifest version: " << data["version"] << std::endl;
        }
        
        // Parse assets
        if (!data.contains("assets") || !data["assets"].is_array()) {
            std::cerr << "Invalid JSON: missing 'assets' array" << std::endl;
            return false;
        }
        
        m_assetTree.clear();
        
        for (const auto& jsonItem : data["assets"]) {
            AssetItem item;
            parseAssetItem(jsonItem, item);
            m_assetTree.push_back(item);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading assets from JSON: " << e.what() << std::endl;
        return false;
    }
}

void Viewer::parseAssetItem(const json& jsonItem, AssetItem& item) {
    // Required fields
    if (jsonItem.contains("name")) {
        item.name = jsonItem["name"];
    }
    if (jsonItem.contains("type")) {
        item.type = jsonItem["type"];
    }
    
    // Optional fields
    if (jsonItem.contains("path")) {
        item.path = jsonItem["path"];
    }
    
    // Parse children recursively
    if (jsonItem.contains("children") && jsonItem["children"].is_array()) {
        for (const auto& jsonChild : jsonItem["children"]) {
            AssetItem child;
            parseAssetItem(jsonChild, child);
            item.children.push_back(child);
        }
    }
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
    
    // Get main viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 menuBarSize = ImVec2(0, ImGui::GetFrameHeight());
    
    // Asset tree panel - left side, fixed width
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarSize.y));
    ImGui::SetNextWindowSize(ImVec2(300, viewport->Size.y - menuBarSize.y));
    renderAssetTree();
    
    // Viewport - fills remaining space
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 300, viewport->Pos.y + menuBarSize.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x - 300, viewport->Size.y - menuBarSize.y));
    renderViewport();
    
    // Config dialog
    if (m_showConfigDialog) {
        renderConfigDialog();
    }
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Viewer::setObjectToRender(const std::string& objectName) {
    std::cout << "Setting object to render: " << objectName << std::endl;
    // For now, just select the first asset to show the dummy cube
    // In the future, this would load the actual specified object
    if (!m_assetTree.empty()) {
        m_selectedAsset = &m_assetTree[0];
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
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | 
                                    ImGuiWindowFlags_NoMove | 
                                    ImGuiWindowFlags_NoResize;
    
    ImGui::Begin("Assets", nullptr, window_flags);
    
    ImGui::Text("Asset Browser");
    ImGui::Separator();
    
    // Recursive function to render tree
    std::function<void(AssetItem&)> renderTreeNode = [&](AssetItem& item) {
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | 
                                       ImGuiTreeNodeFlags_OpenOnDoubleClick;
        
        if (item.children.empty()) {
            node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }
        
        if (item.isSelected) {
            node_flags |= ImGuiTreeNodeFlags_Selected;
        }
        
        // Icon based on type
        const char* icon = "📄";
        if (item.type == "folder") icon = "📁";
        else if (item.type == "model") icon = "🧊";
        else if (item.type == "mesh") icon = "▲";
        else if (item.type == "bones") icon = "🦴";
        
        bool node_open = ImGui::TreeNodeEx(item.name.c_str(), node_flags, 
                                          "%s %s", icon, item.name.c_str());
        
        // Handle selection
        if (ImGui::IsItemClicked()) {
            // Clear all selections (simple selection for now)
            std::function<void(AssetItem&)> clearSelection = [&](AssetItem& i) {
                i.isSelected = false;
                for (auto& child : i.children) {
                    clearSelection(child);
                }
            };
            for (auto& root : m_assetTree) {
                clearSelection(root);
            }
            
            item.isSelected = true;
            
            // Only set selected asset for renderable types (not folders)
            if (item.type != "folder") {
                m_selectedAsset = &item;
                std::cout << "Selected: " << item.name << " (" << item.type << ")" << std::endl;
            } else {
                m_selectedAsset = nullptr;
                std::cout << "Selected folder: " << item.name << std::endl;
            }
        }
        
        // Render children
        if (node_open && !item.children.empty()) {
            for (auto& child : item.children) {
                renderTreeNode(child);
            }
            ImGui::TreePop();
        }
    };
    
    // Render root items
    for (auto& item : m_assetTree) {
        renderTreeNode(item);
    }
    
    ImGui::End();
}

void Viewer::renderViewport() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | 
                                    ImGuiWindowFlags_NoMove | 
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoTitleBar;
    
    ImGui::Begin("Viewport", nullptr, window_flags);
    
    // Check if mouse is hovering over this window's content area
    m_isViewportHovered = ImGui::IsWindowHovered();
    
    // Get viewport size
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        // Render 3D scene if an asset is selected
        if (m_selectedAsset) {
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
        if (m_selectedAsset && m_config.showCameraInfo) {
            ImGui::SetCursorPos(ImVec2(10, 10));
            ImGui::BeginChild("CamInfo", ImVec2(250, 50), true, ImGuiWindowFlags_NoScrollbar);
            glm::vec3 camPos = m_camera->getPosition();
            ImGui::Text("Camera: %.1f, %.1f, %.1f", camPos.x, camPos.y, camPos.z);
            ImGui::Text("Distance: %.1f", m_camera->getDistance());
            ImGui::EndChild();
        }
            
        // Controls hint (only when object selected and enabled)
        if (m_selectedAsset && m_config.showControls) {
            ImGui::SetCursorPos(ImVec2(10, viewportSize.y - 80));
            ImGui::BeginChild("Controls", ImVec2(250, 70), true, ImGuiWindowFlags_NoScrollbar);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Right Mouse: Rotate");
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Middle Mouse: Pan");
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Mouse Wheel: Zoom");
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Home: Reset Camera");
            ImGui::EndChild();
        }
    }
    
    ImGui::End();
}

void Viewer::shutdown() {
    std::cout << "Shutting down viewer..." << std::endl;
    
    // Cleanup framebuffer
    deleteFramebuffer();
    
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
