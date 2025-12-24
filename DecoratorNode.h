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

	bool checkCondition(Agent& agent) override {
		return false;
	}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		// 1. まず条件をチェックする
		if (checkCondition(agent)) {
			// 2. 条件が通れば、子ノードをtickし、その結果をそのまま返す
			if (child) {
				return child->tick(agent, opponent);
			}
			// 子がいない場合はとりあえずSUCCESS
			return status = NodeStatus::SUCCESS;
		}

		// 3. 条件が通らなければ、即座にFAILUREを返す
		return status = NodeStatus::FAILURE;
	}

};