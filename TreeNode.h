#pragma once

// =================================================================================================
// 3. GUIツリービューの実装
// =================================================================================================

struct TreeNode {
	std::string name;
	std::vector<TreeNode> children;
	bool isExpanded = true;
	Color col = GRAY;
	std::shared_ptr<Node> btNode; // 対応するBTノード

	TreeNode(std::shared_ptr<Node> associatedBtNode) : btNode(associatedBtNode) {}
};

extern void InitializeGuiTree(TreeNode& guiNode, const std::shared_ptr<Node>& btNode);
extern void UpdateGuiTree(TreeNode& guiNode);
extern void DrawTreeView(TreeNode& node, int& currentY, int indent);

