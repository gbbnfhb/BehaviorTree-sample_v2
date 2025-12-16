#pragma once
#include "node.h"

// WaitNode: Žw’èŽžŠÔ‘Ò‹@‚·‚é
class WaitNode : public Node {
private:
	float duration;
	float start_time = -1.0f;
public:
	WaitNode(std::string name, float duration_seconds) : Node(name), duration(duration_seconds) {}

	void reset() override {
		Node::reset();
		start_time = -1.0f;
	}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		if (start_time < 0) {
			start_time = GetTime();
		}

		if (GetTime() - start_time >= duration) {
			return status = NodeStatus::SUCCESS;
		}
		return status = NodeStatus::RUNNING;
	}
};