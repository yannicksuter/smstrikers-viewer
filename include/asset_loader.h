#ifndef SMSTRIKERS_ASSET_LOADER_H
#define SMSTRIKERS_ASSET_LOADER_H

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace SMStrikers {

struct AssetLoadResult {
    bool success = false;
    std::string message;
    uintmax_t fileSize = 0;
    std::shared_ptr<struct TextureBundle> textureBundle;
};

struct TextureImage {
    uint32_t hash = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t format = 0;
    uint32_t numLevels = 0;
    uint32_t paletteEntries = 0;
    std::vector<uint8_t> rgba;
};

struct TextureBundle {
    std::vector<TextureImage> textures;
};

class IAssetLoader {
public:
    virtual ~IAssetLoader() = default;
    virtual AssetLoadResult load(const std::filesystem::path& path) const = 0;
    virtual const char* name() const = 0;
    virtual const char* extension() const = 0;
};

class AssetLoaderRegistry {
public:
    AssetLoaderRegistry();
    const IAssetLoader* getLoaderForExtension(const std::string& extension) const;

private:
    void registerLoader(std::unique_ptr<IAssetLoader> loader);

    std::vector<std::unique_ptr<IAssetLoader>> m_loaders;
    std::unordered_map<std::string, size_t> m_loaderByExtension;
};

} // namespace SMStrikers

#endif // SMSTRIKERS_ASSET_LOADER_H
