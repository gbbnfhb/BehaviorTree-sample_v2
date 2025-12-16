#pragma once
#include "CompositeNode.h"

// Sequence: 子を順番に実行し、一つでもFAILUREかRUNNINGになれば、そこで止める
class Sequence : public CompositeNode {
public:
	Sequence(std::string name, std::vector<std::shared_ptr<Node>> children_nodes) 
		: CompositeNode(name, std::move(children_nodes)) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		size_t start_index = (running_child_index != -1) ? running_child_index : 0;

		for (size_t i = start_index; i < children.size(); ++i) {
			NodeStatus child_status = children[i]->tick(agent, opponent);

			if (child_status != NodeStatus::SUCCESS) {
				if (child_status == NodeStatus::RUNNING) {
					running_child_index = i;
				} else {
					running_child_index = -1;
				}
				return status = child_status;
			}
		}
		running_child_index = -1;
		return status = NodeStatus::SUCCESS;
	}
};