#ifndef SMSTRIKERS_VIEWER_H
#define SMSTRIKERS_VIEWER_H

#include <string>

namespace SMStrikers {

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
     * @return true if initialization was successful
     */
    bool initialize(int width, int height, const std::string& title);

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
    bool m_initialized;
    int m_windowWidth;
    int m_windowHeight;
    std::string m_windowTitle;
    
    // Window and OpenGL context will be added here
    // void* m_window; // GLFW or SDL window
};

} // namespace SMStrikers

#endif // SMSTRIKERS_VIEWER_H
