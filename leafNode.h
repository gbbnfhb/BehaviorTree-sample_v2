#pragma once
#include "node.h"
#include "raymath.h"

// --- 具体的な行動ノード (Leaf Node) ---

// IsEnemyNearNode: 敵が近くにいるか判定
class IsEnemyNearNode : public Node {
private:
	float distance_threshold;
public:
	IsEnemyNearNode(std::string name, float distance) : Node(name), distance_threshold(distance) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		float dist = Vector2Distance(agent.position, opponent.position);
		return status = (dist < distance_threshold) ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
	}
};

// MoveNode: ターゲットに向かって移動
class MoveNode : public Node {
public:
	MoveNode(std::string name) : Node(name) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		float dist = Vector2Distance(agent.position, agent.targetPosition);
		if (dist < 2.0f) {
			return status = NodeStatus::SUCCESS;
		}

		Vector2 direction = Vector2Subtract(agent.targetPosition, agent.position);
		direction = Vector2Normalize(direction);
		agent.position = Vector2Add(agent.position, Vector2Scale(direction, agent.speed * GetFrameTime()));

		return status = NodeStatus::RUNNING;
	}
};

// SetRandomTargetNode: ランダムな目標地点を設定
class SetRandomTargetNode : public Node {
public:
	SetRandomTargetNode(std::string name) : Node(name) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		agent.targetPosition = { (float)GetRandomValue(50, 750), (float)GetRandomValue(50, 550) };
		return status = NodeStatus::SUCCESS;
	}
};