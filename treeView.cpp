#include "raylib.h"
#include "raymath.h"

//#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <cmath>

#include "node.h"
#include "treeView.h"


// 初期化：BTの構造をGUIツリーに一度だけコピーする
void InitializeGuiTree(TreeNode& guiNode, const std::shared_ptr<Node>& btNode) {
	auto childrenBtNodes = btNode->getChildren();
	for (const auto& childBtNode : childrenBtNodes) {
		TreeNode newGuiChild(childBtNode);
		InitializeGuiTree(newGuiChild, childBtNode);
		guiNode.children.push_back(newGuiChild);
	}
}

// 更新：毎フレーム、GUIツリーの状態（テキストと色）を更新する
void UpdateGuiTree(TreeNode& guiNode) {
	if (guiNode.btNode) {
		guiNode.name = guiNode.btNode->getStatusText();
		switch (guiNode.btNode->getStatus()) {
		case NodeStatus::SUCCESS: guiNode.col = BLUE; break;
		case NodeStatus::FAILURE: guiNode.col = RED; break;
		case NodeStatus::RUNNING: guiNode.col = LIME; break;
		default:                  guiNode.col = GRAY; break;
		}
	}
	for (TreeNode& child : guiNode.children) {
		UpdateGuiTree(child);
	}
}

// 描画：GUIツリーを描画する
void DrawTreeView(TreeNode& node, int& currentY, int indent) {
	int nodeHeight = 20;
	int nodeX = 810 + indent * 20;

	// 展開/折りたたみボタン
	if (!node.children.empty()) {
		if (GuiButton(Rectangle{(float)nodeX, (float)currentY, 15, (float)nodeHeight}, node.isExpanded ? ">" : "<"))
		{
			node.isExpanded = !node.isExpanded;
		}
		nodeX += 20;
	}

	// ノード名（色付き）
	DrawText(node.name.c_str(), nodeX, currentY, 20, node.col);

	currentY += nodeHeight;

	// 子ノードの描画
	if (node.isExpanded) {
		for (TreeNode& child : node.children) {
			DrawTreeView(child, currentY, indent + 1);
		}
	}
}
