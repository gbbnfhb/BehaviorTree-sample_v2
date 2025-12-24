#pragma once

class ParallelNode : public CompositeNode {
private:
	Policy successPolicy;
	Policy failurePolicy;

public:
	ParallelNode(std::string name, Policy successPol, Policy failurePol, const std::vector<std::shared_ptr<Node>>& nodes)
		: CompositeNode(name, nodes), successPolicy(successPol), failurePolicy(failurePol) {}

	// 文字列からポリシーを設定するコンストラクタ（Luaから使いやすいように）
	ParallelNode(std::string name, const std::string& successPolStr, const std::string& failurePolStr, const std::vector<std::shared_ptr<Node>>& nodes)
		: CompositeNode(name, nodes) {

		if (successPolStr == "RequireOne") successPolicy = Policy::RequireOne;
		else if (successPolStr == "RequireAll") successPolicy = Policy::RequireAll;
		// デフォルトはRequireOne
		else successPolicy = Policy::RequireOne; 

		if (failurePolStr == "RequireOne") failurePolicy = Policy::RequireOne;
		else if (failurePolStr == "RequireAll") failurePolicy = Policy::RequireAll;
		// デフォルトはRequireOne
		else failurePolicy = Policy::RequireOne;
	}

	~ParallelNode() override{
	}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		size_t successCount = 0;
		size_t failureCount = 0;

		// 全ての子ノードを毎フレームtickする
		for (const auto& child : children) {
			NodeStatus childStatus = child->tick(agent, opponent);

			switch (childStatus) {
			case NodeStatus::SUCCESS:
			successCount++;
			// 成功ポリシーがRequireOneなら、即座にSUCCESSを返す
			if (successPolicy == Policy::RequireOne) {
				reset(); // 完了したのでリセット
				return status = NodeStatus::SUCCESS;
			}
			break;

			case NodeStatus::FAILURE:
			failureCount++;
			// 失敗ポリシーがRequireOneなら、即座にFAILUREを返す
			if (failurePolicy == Policy::RequireOne) {
				reset(); // 完了したのでリセット
				return status = NodeStatus::FAILURE;
			}
			break;

			case NodeStatus::RUNNING:
			// RUNNINGの子がいる場合は、Parallel全体もRUNNINGの可能性がある
			break;

			default:
			// INVALIDなど
			break;
			}
		}

		// ループを抜けた後の最終判断
		// 成功ポリシーがRequireAllで、全て成功した場合
		if (successPolicy == Policy::RequireAll && successCount == children.size()) {
			reset();
			return status = NodeStatus::SUCCESS;
		}

		// 失敗ポリシーがRequireAllで、全て失敗した場合
		if (failurePolicy == Policy::RequireAll && failureCount == children.size()) {
			reset();
			return status = NodeStatus::FAILURE;
		}

		// 上記の完了条件を満たさなければ、まだ実行中
		return status = NodeStatus::RUNNING;
	}

};