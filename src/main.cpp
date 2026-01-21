#include "viewer.h"
#include <iostream>
#include <cstdlib>

/**
 * Super Mario Strikers Asset Viewer
 * 
 * IMPORTANT: This application requires a legitimate copy of 
 * Super Mario Strikers (Nintendo GameCube, 2005).
 * No game assets are included with this software.
 */

void printBanner() {
    std::cout << "========================================" << std::endl;
    std::cout << " Super Mario Strikers - Asset Viewer" << std::endl;
    std::cout << " Version 0.1.0 (Development)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "DISCLAIMER:" << std::endl;
    std::cout << "This software does NOT include any game assets." << std::endl;
    std::cout << "You must own a legitimate copy of the game." << std::endl;
    std::cout << std::endl;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h     Show this help message" << std::endl;
    std::cout << "  --version, -v  Show version information" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if (arg == "--version" || arg == "-v") {
            std::cout << "Super Mario Strikers Viewer v0.1.0" << std::endl;
            return EXIT_SUCCESS;
        }
    }
    
    printBanner();
    
    // Create and initialize viewer
    SMStrikers::Viewer viewer;
    
    if (!viewer.initialize(1280, 720, "Super Mario Strikers Viewer")) {
        std::cerr << "Failed to initialize viewer!" << std::endl;
        return EXIT_FAILURE;
    }
    
    // Run the application
    int result = viewer.run();
    
    // Cleanup is handled by destructor
    return result;
}
