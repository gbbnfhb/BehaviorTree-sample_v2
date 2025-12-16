#pragma once
#include "node.h"

// 1つの子を持つノード (Repeater) の親
class DecoratorNode : public Node {
protected:
	std::shared_ptr<Node> child;

public:
	DecoratorNode(std::string name, std::shared_ptr<Node> child_node) 
		: Node(name), child(std::move(child_node)) {}

	void reset() override {
		Node::reset();
		if (child) child->reset();
	}

	std::vector<std::shared_ptr<Node>> getChildren() const override {
		if (child) return {child};
		return {};
	}
};