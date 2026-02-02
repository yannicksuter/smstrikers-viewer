#include "asset_tree.h"
#include <algorithm>
#include <cctype>
#include <iostream>

namespace SMStrikers {

namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

bool isFolderKind(AssetKind kind) {
    return kind == AssetKind::Folder;
}

bool nodeSort(const AssetNode& a, const AssetNode& b) {
    if (isFolderKind(a.kind) != isFolderKind(b.kind)) {
        return isFolderKind(a.kind);
    }
    return a.name < b.name;
}

const AssetNode* findNodeRecursive(const std::vector<AssetNode>& nodes, const std::string& relativePath) {
    for (const auto& node : nodes) {
        if (node.relativePath == relativePath) {
            return &node;
        }
        if (!node.children.empty()) {
            const AssetNode* found = findNodeRecursive(node.children, relativePath);
            if (found) {
                return found;
            }
        }
    }
    return nullptr;
}

} // namespace

bool AssetTreeModel::loadFromFilesystem(const std::string& rootPath) {
    m_roots.clear();
    m_stats = {};
    m_rootPathString = rootPath;
    m_rootPath = std::filesystem::path(rootPath);

    if (rootPath.empty()) {
        std::cout << "Assets root is empty." << std::endl;
        return false;
    }

    if (!std::filesystem::exists(m_rootPath) || !std::filesystem::is_directory(m_rootPath)) {
        std::cout << "Assets root not found: " << rootPath << std::endl;
        return false;
    }

    try {
        for (const auto& entry : std::filesystem::directory_iterator(m_rootPath)) {
            if (entry.is_directory()) {
                AssetNode node = buildNode(entry);
                if (!node.children.empty()) {
                    m_roots.push_back(std::move(node));
                }
                continue;
            }

            if (entry.is_regular_file()) {
                std::string ext = toLower(entry.path().extension().string());
                AssetKind kind = assetKindFromExtension(ext);
                if (!isLoadable(kind)) {
                    continue;
                }
                m_roots.push_back(buildNode(entry));
                continue;
            }
        }
        std::sort(m_roots.begin(), m_roots.end(), nodeSort);

        for (const auto& node : m_roots) {
            accumulateStats(node);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning assets root: " << e.what() << std::endl;
        return false;
    }

    return true;
}

AssetNode AssetTreeModel::buildNode(const std::filesystem::directory_entry& entry) {
    AssetNode node;
    node.name = entry.path().filename().string();

    if (entry.is_directory()) {
        node.kind = AssetKind::Folder;
        node.relativePath = std::filesystem::relative(entry.path(), m_rootPath).generic_string();
        buildChildren(entry.path(), node.children);
    } else if (entry.is_regular_file()) {
        std::string ext = toLower(entry.path().extension().string());
        node.kind = assetKindFromExtension(ext);
        node.relativePath = std::filesystem::relative(entry.path(), m_rootPath).generic_string();
    } else {
        node.kind = AssetKind::File;
        node.relativePath = std::filesystem::relative(entry.path(), m_rootPath).generic_string();
    }

    return node;
}

void AssetTreeModel::buildChildren(const std::filesystem::path& dirPath, std::vector<AssetNode>& outChildren) {
    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (entry.is_directory()) {
            AssetNode child = buildNode(entry);
            if (!child.children.empty()) {
                outChildren.push_back(std::move(child));
            }
            continue;
        }

        if (entry.is_regular_file()) {
            std::string ext = toLower(entry.path().extension().string());
            AssetKind kind = assetKindFromExtension(ext);
            if (!isLoadable(kind)) {
                continue;
            }
            AssetNode child = buildNode(entry);
            outChildren.push_back(std::move(child));
            continue;
        }
    }
    std::sort(outChildren.begin(), outChildren.end(), nodeSort);
}

void AssetTreeModel::accumulateStats(const AssetNode& node) {
    m_stats.nodeCount++;
    if (node.kind == AssetKind::Folder) {
        m_stats.folderCount++;
    } else {
        m_stats.fileCount++;
    }
    if (isLoadable(node.kind)) {
        m_stats.loadableCount++;
    }
    for (const auto& child : node.children) {
        accumulateStats(child);
    }
}

const AssetNode* AssetTreeModel::findByPath(const std::string& relativePath) const {
    if (relativePath.empty()) {
        return nullptr;
    }
    return findNodeRecursive(m_roots, relativePath);
}

bool isLoadable(AssetKind kind) {
    return kind == AssetKind::TextureBundle || kind == AssetKind::ModelBundle;
}

const char* assetKindLabel(AssetKind kind) {
    switch (kind) {
    case AssetKind::Folder:
        return "Folder";
    case AssetKind::TextureBundle:
        return "Texture Bundle (.glt)";
    case AssetKind::ModelBundle:
        return "Model Bundle (.glg)";
    case AssetKind::File:
    default:
        return "File";
    }
}

const char* assetKindShortLabel(AssetKind kind) {
    switch (kind) {
    case AssetKind::Folder:
        return "folder";
    case AssetKind::TextureBundle:
        return "glt";
    case AssetKind::ModelBundle:
        return "glg";
    case AssetKind::File:
    default:
        return "file";
    }
}

AssetKind assetKindFromExtension(const std::string& extension) {
    if (extension == ".glt") {
        return AssetKind::TextureBundle;
    }
    if (extension == ".glg") {
        return AssetKind::ModelBundle;
    }
    return AssetKind::File;
}

} // namespace SMStrikers
