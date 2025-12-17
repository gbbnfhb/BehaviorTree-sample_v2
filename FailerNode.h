#pragma once
#include "node.h"


class FailerNode : public Node {
public:
	FailerNode(std::string name) : Node(name) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		// どんな状況でも、常にFAILUREを返す
		return status = NodeStatus::FAILURE;
	}
};

class SuccessNode : public Node {
public:
	SuccessNode(std::string name) : Node(name) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		// どんな状況でも、常にSUCCESSを返す
		return status = NodeStatus::SUCCESS;
	}
};

// 子ノードの結果を反転させるデコレーターノード
class InverterNode : public Node {
protected:
	std::shared_ptr<Node> child;

public:
	InverterNode(std::string name, std::shared_ptr<Node> childNode) 
		: Node(name), child(childNode) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		if (!child) {
			return status = NodeStatus::FAILURE;
		}

		NodeStatus childStatus = child->tick(agent, opponent);

		switch (childStatus) {
		case NodeStatus::SUCCESS:
		status = NodeStatus::FAILURE;
		break;
		case NodeStatus::FAILURE:
		status = NodeStatus::SUCCESS;
		break;
		case NodeStatus::RUNNING:
		status = NodeStatus::RUNNING; // RUNNINGはそのまま
		break;
		default:
		status = NodeStatus::INVALID;
		break;
		}
		return status;
	}

	void reset() override {
		Node::reset();
		if (child) {
			child->reset();
		}
	}

	std::vector<std::shared_ptr<Node>> getChildren() const override {
		if (child) {
			return { child };
		}
		return {};
	}
};
