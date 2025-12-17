#pragma once
// =================================================================================================
// 1. ビヘイビアツリー (BT) の基本クラス
// =================================================================================================

// ノードの状態
enum class NodeStatus {
	INVALID,
	SUCCESS,
	FAILURE,
	RUNNING,
};

enum class Policy {
	RequireOne, // 一つでも条件を満たせばOK
	RequireAll  // 全てが条件を満たす必要がある
};

// 状態を文字列に変換するヘルパー
inline const char* StatusToString(NodeStatus status) {
	switch (status) {
	case NodeStatus::SUCCESS: return "SUCCESS";
	case NodeStatus::FAILURE: return "FAILURE";
	case NodeStatus::RUNNING: return "RUNNING";
	default:                  return "INVALID";
	}
}

// エージェントのダミークラス
struct Agent {
	Vector2 position = { 400, 300 };
	Vector2 targetPosition = { 400, 300 };
	float speed = 100.0f; // 1秒あたりのピクセル移動量
};

// 全てのノードの基底クラス
class Node {
protected:
	NodeStatus status = NodeStatus::INVALID;
	std::string name;

public:
	Node(std::string name) : name(name) {}
	virtual ~Node() = default;

	virtual NodeStatus tick(Agent& agent, const Agent& opponent) = 0;

	virtual void reset() {
		status = NodeStatus::INVALID;
	}

	NodeStatus getStatus() const { return status; }

	std::string getStatusText() const {
		return name + ": " + StatusToString(status);
	}

	virtual std::vector<std::shared_ptr<Node>> getChildren() const {
		return {};
	}
};

#include "CompositeNode.h"
#include "DecoratorNode.h"
#include "SetTargetToOpponentNode.h"
#include "Selector.h"
#include "Sequence.h"
#include "Repeater.h"
#include "WaitNode.h"
#include "ParallelNode.h"
#include "FailerNode.h"

#include "leafNode.h"