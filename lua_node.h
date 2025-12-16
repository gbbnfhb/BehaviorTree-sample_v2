#pragma once
#include "node.h"

class LuaNode : public Node {
private:
	sol::protected_function tick_function; // 実行するLua関数
	sol::state_view lua;                   // Luaの実行環境への参照

public:
	LuaNode(std::string name, sol::protected_function func, sol::state_view lua_state)
		: Node(name), tick_function(func), lua(lua_state) {}

	// NodeStatusをLuaで扱えるように、文字列と相互変換する
	static NodeStatus StringToStatus(const std::string& s) {
		if (s == "SUCCESS") return NodeStatus::SUCCESS;
		if (s == "FAILURE") return NodeStatus::FAILURE;
		if (s == "RUNNING") return NodeStatus::RUNNING;
		return NodeStatus::INVALID;
	}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		if (!tick_function.valid()) {
			return status = NodeStatus::FAILURE;
		}

		// Lua関数を呼び出す
		// 引数としてagentとopponentを渡す（これらもLuaに公開する必要がある）
		auto result = tick_function(agent, opponent);

		if (result.valid()) {
			// 戻り値（文字列）をNodeStatusに変換
			return status = StringToStatus(result.get<std::string>());
		} else {
			// Lua関数実行中にエラーが発生した場合
			sol::error err = result;
			std::cerr << "LuaNode '" << name << "' execution error: " << err.what() << std::endl;
			return status = NodeStatus::FAILURE;
		}
	}
};

extern sol::state Lua;
