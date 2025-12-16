#pragma once
#include "node.h"

// 複数の子を持つノード (Selector, Sequence) の親
class CompositeNode : public Node {
protected:
	std::vector<std::shared_ptr<Node>> children;
	int running_child_index = -1; // 実行中の子を記憶

public:
	CompositeNode(std::string name, std::vector<std::shared_ptr<Node>> children_nodes) 
		: Node(name), children(std::move(children_nodes)) {}

	void reset() override {
		Node::reset();
		running_child_index = -1;
		for (const auto& child : children) {
			child->reset();
		}
	}

	std::vector<std::shared_ptr<Node>> getChildren() const override {
		return children;
	}
};
