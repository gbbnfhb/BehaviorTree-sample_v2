#pragma once
#include <utility> // std::forward のために必要
#include"Blackboard.h"
// --- 前方宣言 ---
class Agent;

// --- ビヘイビアツリー基本コンポーネント ---
enum class NodeStatus { INVALID, SUCCESS, FAILURE, RUNNING };

std::string StatusToString(NodeStatus status) {
	switch (status) {
	case NodeStatus::SUCCESS: return "SUCCESS";
	case NodeStatus::FAILURE: return "FAILURE";
	case NodeStatus::RUNNING: return "RUNNING";
	case NodeStatus::INVALID: return "INVALID";
	}
	return "UNKNOWN";
}

class Node {
protected:
	NodeStatus gui_status = NodeStatus::INVALID;
public:
	virtual ~Node() = default;
	virtual NodeStatus tick(Agent& agent, const Agent& opponent) = 0;
	virtual std::string getStatusText() const = 0;
	virtual void getTreeViewText(std::string& text, const std::string& indent) const {
		text += indent + getStatusText() + "\n";
	}

	NodeStatus getStatus() const { return gui_status; }

	virtual void gui_status_reset() {
		// 自分のステータスをリセットする
		gui_status = NodeStatus::INVALID;
	}

	virtual const std::vector<std::shared_ptr<Node>> getChildren() const {
		// LeafNodeなど、子を持たないノードは空のベクターを返す
		return {};
	}
/*
	// 子ノードのリストを返す (BranchNode用)
	virtual const std::vector<std::shared_ptr<Node>>& getChildren() const {
		// BranchNode以外は空のリストを返す
		static const std::vector<std::shared_ptr<Node>> empty;
		return empty;
	}
*/
};

class BranchNode : public Node {
protected:
	std::vector<std::shared_ptr<Node>> children;
public:
	BranchNode(std::vector<std::shared_ptr<Node>> children_nodes) : children(std::move(children_nodes)) {}

	void getTreeViewText(std::string& text, const std::string& indent) const override {
		text += indent + getStatusText() + "\n";
		for (const auto& child : children) {
			child->getTreeViewText(text, indent + "  ");
		}
	}

	void gui_status_reset() override {
		// まず自分のステータスをリセット (基底クラスのメソッドを呼ぶ)
		Node::gui_status_reset();

		// すべての子に対しても再帰的にgui_status_reset()を呼び出す
		for (const auto& child : children) {
			child->gui_status_reset();
		}
	}
/*
	virtual std::string getStatusText() const {
		return "BranchNode: " + StatusToString(gui_status);
	}
*/
	virtual std::string getBranchTypeName() const = 0;

	// getStatusTextはBranchNodeで実装してしまう
	std::string getStatusText() const override {
		// "Selector (Branch): RUNNING" のように表示する
		return " (Branch): " + getBranchTypeName() +  StatusToString(gui_status);
	}

	// getChildrenをオーバーライドして、実際の子ノードリストを返す
	const std::vector<std::shared_ptr<Node>> getChildren() const override {
		return children;
	}
};

class Sequence : public BranchNode {
public:
	using BranchNode::BranchNode;
	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		for (const auto& child : children) {
			NodeStatus child_status = child->tick(agent, opponent);
			if (child_status != NodeStatus::SUCCESS) {
				gui_status = child_status;
				return gui_status;
			}
		}
		gui_status = NodeStatus::SUCCESS;
		return gui_status;
	}

	std::string getBranchTypeName() const override {
		return "Sequence";
	}
/*
	virtual std::string getDebugText() const {
		return "Sequence: " + StatusToString(gui_status);
	}

	std::string getStatusText() const override { return "Sequence: " + StatusToString(gui_status); }
*/
};

class Selector : public BranchNode {
public:
	using BranchNode::BranchNode;
	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		for (const auto& child : children) {
			NodeStatus child_status = child->tick(agent, opponent);

			// 子がFAILUREを返さなかった場合（SUCCESS または RUNNING の場合）
			if (child_status != NodeStatus::FAILURE) {
				// そのステータスを自分自身のステータスとして、すぐに親に返す
				gui_status = child_status;
				return gui_status;
			}
		}
		// すべての子がFAILUREを返した場合のみ、FAILUREを返す
		gui_status = NodeStatus::FAILURE;
		return gui_status;
	}

	std::string getBranchTypeName() const override {
		return "Selector";
	}


};

// --- Agentクラス ---
class Agent {
public:
	Vector2 position;
	Color color;
	float speed = 1.0f;
	std::shared_ptr<Node> behavior_tree = nullptr;
	std::shared_ptr<Blackboard> blackboard = nullptr; // ブラックボードへのポインタを追加


	Agent(float x, float y, Color c, std::shared_ptr<Node> bt = nullptr)
		: position{x, y}, color(c), behavior_tree(std::move(bt)) {
		blackboard = std::make_shared<Blackboard>();
	}

	void update(const Agent& opponent) {
		// 1. ツリー全体のステータスをリセットする
		behavior_tree->gui_status_reset();

		// 2. その後で、ツリーの評価を開始する
		if (behavior_tree) {
			behavior_tree->tick(*this, opponent);
		}
	}

	void draw() const {
		DrawCircleV(position, 15.0f, color);
	}
};

// --- Lua連携ノード ---
class LuaNode : public Node {
private:
	// sol::state_view has no default constructor, store as "lua_state" and initialize in ctor
	sol::state_view lua_state;
	sol::table self; // Lua側のノードインスタンス
	std::string node_name;
	std::string class_name;

	// 可変長引数を処理するためのプライベートな初期化ヘルパー関数
	template<typename... Args>
	void initialize(sol::state_view lua,
		const std::string& script_path,
		const std::string& class_name,
		Args&&... args)
	{
		try {
			lua.safe_script_file(script_path);
			sol::table lua_class = lua[class_name];
			if (lua_class.valid()) {
				self = lua_class["new"](lua_class, std::forward<Args>(args)...);
			} else {
				std::cerr << "Lua Error: Class '" << class_name << "' not found in " << script_path << std::endl;
			}
		} catch (const sol::error& e) {
			std::cerr << "Lua Error in LuaNode constructor: " << e.what() << std::endl;
		}
	}

public:
	// 公開コンストラクタ：可変長引数テンプレートにする
    template<typename... Args>
    LuaNode(sol::state_view lua,
        const std::string& script_path,
        const std::string& class_name,
        Args&&... args)
        : lua_state(lua), node_name(class_name)
    {
        // プライベートな初期化関数を呼び出す
        initialize(lua_state, script_path, class_name, std::forward<Args>(args)...);
    }

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		if (!self.valid()) return NodeStatus::FAILURE;
		try {
			sol::object result = self["tick"](self, std::ref(agent), std::ref(opponent));
			if (result.is<std::string>()) {
				std::string status_str = result.as<std::string>();
				if (status_str == "SUCCESS") { gui_status = NodeStatus::SUCCESS; return gui_status; }
				if (status_str == "RUNNING") { gui_status = NodeStatus::RUNNING; return gui_status; }
			}
		} catch (const sol::error& e) {
			std::cerr << "Lua Error in tick: " << e.what() << std::endl;
		}
		gui_status = NodeStatus::FAILURE;
		return gui_status;
	}

	// getStatusTextとgetTreeViewTextを実装する
	std::string getStatusText() const override {
		if (!self.valid()) return node_name + ": [INVALID]";
		try {
			// Lua側のgetStatusTextを呼び出す
			sol::function get_status = self["getStatusText"];
			if (get_status.valid()) {
				return get_status(self).get<std::string>();
			}
		} catch (const sol::error& e) {
			std::cerr << "Lua Error in getStatusText: " << e.what() << std::endl;
		}
		return node_name + ": " + StatusToString(gui_status);
	}
/*
	std::string getStatusText() const override {
		// class_nameはコンストラクタで保存しておく
		return "Lua: " + class_name + " -> " + StatusToString(gui_status);
	}
*/
};
