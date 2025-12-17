#pragma once
#include "node.h"
#include "sol3/sol.hpp"

class LuaCoroutineNode : public Node {
private:
	sol::coroutine co; // Luaのコルーチンオブジェクトを保持
	sol::state_view lua;
	sol::protected_function tick_function;

public:
	LuaCoroutineNode(std::string name, sol::protected_function func, sol::state_view lua_state)
		: Node(name), tick_function(func), lua(lua_state) {}

	void reset() override {
		Node::reset();
		co = sol::lua_nil; // コルーチンをリセット
	}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		// まだコルーチンが作られていなかったら、関数から生成する
		if (!co) {
			co = sol::coroutine(tick_function);
		}

		// コルーチンを実行（再開）する
		sol::protected_function_result result = co(agent, opponent);

		if (result.valid()) {
			if (co.status() == sol::call_status::yielded) {
				// yieldされた場合（中断）
				return status = NodeStatus::RUNNING;
			} else {
				// returnされた場合（完了）
				std::string status_str = result.get<std::string>();
				reset(); // 完了したのでリセット
				return status = StringToStatus(status_str);
			}
		} else {
			// エラー処理
			reset();
			return status = NodeStatus::FAILURE;
		}
	}
};