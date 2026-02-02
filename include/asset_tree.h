#ifndef SMSTRIKERS_ASSET_TREE_H
#define SMSTRIKERS_ASSET_TREE_H

#include <filesystem>
#include <string>
#include <vector>

namespace SMStrikers {

enum class AssetKind {
    Folder,
    File,
    TextureBundle,
    ModelBundle
};

struct AssetNode {
    std::string name;
    AssetKind kind = AssetKind::File;
    std::string relativePath;
    std::vector<AssetNode> children;
};

struct AssetTreeStats {
    size_t nodeCount = 0;
    size_t folderCount = 0;
    size_t fileCount = 0;
    size_t loadableCount = 0;
};

class AssetTreeModel {
public:
    bool loadFromFilesystem(const std::string& rootPath);
    const std::vector<AssetNode>& roots() const { return m_roots; }
    const std::string& rootPath() const { return m_rootPathString; }
    const AssetTreeStats& stats() const { return m_stats; }
    const AssetNode* findByPath(const std::string& relativePath) const;
    bool hasRoot() const { return !m_rootPathString.empty(); }

private:
    std::filesystem::path m_rootPath;
    std::string m_rootPathString;
    std::vector<AssetNode> m_roots;
    AssetTreeStats m_stats;

    AssetNode buildNode(const std::filesystem::directory_entry& entry);
    void buildChildren(const std::filesystem::path& dirPath, std::vector<AssetNode>& outChildren);
    void accumulateStats(const AssetNode& node);
};

bool isLoadable(AssetKind kind);
const char* assetKindLabel(AssetKind kind);
const char* assetKindShortLabel(AssetKind kind);
AssetKind assetKindFromExtension(const std::string& extension);

} // namespace SMStrikers

#endif // SMSTRIKERS_ASSET_TREE_H
