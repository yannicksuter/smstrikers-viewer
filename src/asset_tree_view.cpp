#include "asset_tree_view.h"
#include <imgui.h>

namespace SMStrikers {

void AssetTreeView::renderTree(const std::vector<AssetNode>& roots, std::string& selectedPath) {
    for (const auto& node : roots) {
        renderTreeNode(node, selectedPath);
    }
}

void AssetTreeView::renderTreeNode(const AssetNode& node, std::string& selectedPath) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (node.children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    if (node.relativePath == selectedPath) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    const char* icon = "[F]";
    if (node.kind == AssetKind::Folder) {
        icon = "[D]";
    } else if (node.kind == AssetKind::ModelBundle) {
        icon = "[M]";
    } else if (node.kind == AssetKind::TextureBundle) {
        icon = "[T]";
    }

    std::string label = std::string(icon) + " " + node.name + "##" + node.relativePath;
    bool open = ImGui::TreeNodeEx(label.c_str(), flags);

    if (ImGui::IsItemClicked()) {
        selectedPath = node.relativePath;
    }

    if (open && !node.children.empty()) {
        for (const auto& child : node.children) {
            renderTreeNode(child, selectedPath);
        }
        ImGui::TreePop();
    }
}

} // namespace SMStrikers
