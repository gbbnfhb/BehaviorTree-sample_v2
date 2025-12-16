#pragma once
#include "DecoratorNode.h"

// Repeater: 子を指定回数繰り返す
class Repeater : public DecoratorNode {
private:
	const int limit;
	int counter = 0;
public:
	Repeater(std::string name, int repeat_limit, std::shared_ptr<Node> child_node)
		: DecoratorNode(name, std::move(child_node)), limit(repeat_limit) {}

	void reset() override {
		DecoratorNode::reset();
		counter = 0;
	}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		if (!child) {
			return status = NodeStatus::SUCCESS;
		}

		// 規定回数にすでに達しているなら、何もせずSUCCESSを返す
		if (limit != -1 && counter >= limit) {
			return status = NodeStatus::SUCCESS;
		}

		// 子を実行する
		NodeStatus child_status = child->tick(agent, opponent);

		// 子が実行中なら、自分もRUNNINGを返してフレームを終了
		if (child_status == NodeStatus::RUNNING) {
			return status = NodeStatus::RUNNING;
		}

		// 子が失敗したら、自分もFAILUREを返して終了
		if (child_status == NodeStatus::FAILURE) {
			return status = NodeStatus::FAILURE;
		}

		// 子が成功した場合 (child_status == NodeStatus::SUCCESS)
		counter++; // カウンターを増やす
		child->reset(); // 子をリセットして、次の繰り返しに備える

		// 規定回数に達したかチェック
		if (limit != -1 && counter >= limit) {
			// 達した -> 自分はSUCCESSを返して完了
			return status = NodeStatus::SUCCESS;
		} else {
			// まだ達していない -> 次のフレームで続きをやるので、今フレームはRUNNINGを返す
			return status = NodeStatus::RUNNING;
		}
	}
};