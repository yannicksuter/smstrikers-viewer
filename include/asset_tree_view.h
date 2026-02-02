#ifndef SMSTRIKERS_ASSET_TREE_VIEW_H
#define SMSTRIKERS_ASSET_TREE_VIEW_H

#include "asset_tree.h"
#include <string>

namespace SMStrikers {

class AssetTreeView {
public:
    void renderTree(const std::vector<AssetNode>& roots, std::string& selectedPath);

private:
    void renderTreeNode(const AssetNode& node, std::string& selectedPath);
};

} // namespace SMStrikers

#endif // SMSTRIKERS_ASSET_TREE_VIEW_H
