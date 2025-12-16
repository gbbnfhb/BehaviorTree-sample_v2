#pragma once
#include "node.h"

// SetTargetToOpponentNode: 目標地点を敵の位置に設定
class SetTargetToOpponentNode : public Node {
public:
	SetTargetToOpponentNode(std::string name) : Node(name) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		agent.targetPosition = opponent.position;
		return status = NodeStatus::SUCCESS;
	}
};