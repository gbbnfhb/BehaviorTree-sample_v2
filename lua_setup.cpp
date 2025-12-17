#include "raylib.h"
#include "raymath.h"

//#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <cmath>

#define SOL_ALL_SAFETIES_ON 1
#include "sol3/sol.hpp"

#include "lua_node.h"

sol::state Lua;



void bind_nodes_to_lua(sol::state& lua) {
	lua.new_usertype<Vector2>("Vector2",
		"x", &Vector2::x,
		"y", &Vector2::y
	);

	lua.new_usertype<Agent>("Agent",
		"position", &Agent::position,
		"targetPosition", &Agent::targetPosition,
		"speed", &Agent::speed
	);

	lua.new_usertype<FailerNode>("Failer",
		sol::no_constructor,
		sol::base_classes, sol::bases<Node>()
	);

	lua.new_usertype<SuccessNode>("Succeeder",
		sol::no_constructor,
		sol::base_classes, sol::bases<Node>()
	);

	// --- 基本的なノードクラスをLuaに登録 ---
	// これにより、Lua側でNodeオブジェクトを扱えるようになる
	lua.new_usertype<Node>("Node", sol::no_constructor);
	lua.new_usertype<CompositeNode>("CompositeNode", sol::no_constructor,	sol::base_classes, sol::bases<Node>());
	lua.new_usertype<DecoratorNode>("DecoratorNode", sol::no_constructor,	sol::base_classes, sol::bases<Node>());

	lua.new_usertype<Selector>("Selector", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<RSelector>("RSelector", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<Sequence>("Sequence", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<Repeater>("Repeater", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<WaitNode>("WaitNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<IsEnemyNearNode>("IsEnemyNearNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<SetTargetToOpponentNode>("SetTargetToOpponentNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<SetRandomTargetNode>("SetRandomTargetNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<MoveNode>("MoveNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<ParallelNode>("Parallel",sol::no_constructor, sol::base_classes, sol::bases<Node, CompositeNode>());
	lua.new_usertype<InverterNode>("Inverter",sol::no_constructor,	sol::base_classes, sol::bases<Node>());


	// --- 各ノードを生成するための「ファクトリ関数」をLuaに公開 ---
	// Luaのグローバル空間に、BTというテーブル（名前空間のようなもの）を作る
	lua["BT"] = lua.create_table();


	// ★★★ LuaNodeを生成するファクトリ関数を追加 ★★★
	lua["BT"]["LuaNode"] = [&](std::string name, sol::protected_function func) -> std::shared_ptr<Node> {
		// LuaNodeのコンストラクタにはsol::state_viewが必要なので、luaオブジェクトを渡す
		return std::static_pointer_cast<Node>(std::make_shared<LuaNode>(name, func, lua));
	};


	lua["BT"]["Selector"] = [](std::string name, sol::table children_table) -> std::shared_ptr<Node> { // ★戻り値の型を明記
		std::vector<std::shared_ptr<Node>> children;
		for (const auto& kv : children_table) {
			children.push_back(kv.second.as<std::shared_ptr<Node>>());
		}
		// ★戻り値を基底クラスのshared_ptrにキャスト
		return std::static_pointer_cast<Node>(std::make_shared<Selector>(name, children));
	};

	lua["BT"]["RSelector"] = [](std::string name, sol::table children_table) -> std::shared_ptr<Node> { // ★戻り値の型を明記
		std::vector<std::shared_ptr<Node>> children;
		for (const auto& kv : children_table) {
			children.push_back(kv.second.as<std::shared_ptr<Node>>());
		}
		// ★戻り値を基底クラスのshared_ptrにキャスト
		return std::static_pointer_cast<Node>(std::make_shared<RSelector>(name, children));
	};

	lua["BT"]["Sequence"] = [](std::string name, sol::table children_table) -> std::shared_ptr<Node> { // ★
		std::vector<std::shared_ptr<Node>> children;
		for (const auto& kv : children_table) {
			children.push_back(kv.second.as<std::shared_ptr<Node>>());
		}
		return std::static_pointer_cast<Node>(std::make_shared<Sequence>(name, children)); // ★
	};

	lua["BT"]["Repeater"] = [](std::string name, int limit, const std::shared_ptr<Node>& child) -> std::shared_ptr<Node> { // ★
		return std::static_pointer_cast<Node>(std::make_shared<Repeater>(name, limit, child)); // ★
	};

	lua["BT"]["Wait"] = [](std::string name, float duration) -> std::shared_ptr<Node> { // ★
		return std::static_pointer_cast<Node>(std::make_shared<WaitNode>(name, duration)); // ★
	};

	lua["BT"]["IsEnemyNear"] = [](std::string name, float distance) -> std::shared_ptr<Node> { // ★
		return std::static_pointer_cast<Node>(std::make_shared<IsEnemyNearNode>(name, distance)); // ★
	};

	// ... 他の全てのファクトリ関数も同様に修正 ...
	lua["BT"]["SetTargetToOpponent"] = [](std::string name) -> std::shared_ptr<Node> { // ★
		return std::static_pointer_cast<Node>(std::make_shared<SetTargetToOpponentNode>(name)); // ★
	};

	lua["BT"]["SetRandomTarget"] = [](std::string name) -> std::shared_ptr<Node> { // ★
		return std::static_pointer_cast<Node>(std::make_shared<SetRandomTargetNode>(name)); // ★
	};

	lua["BT"]["Move"] = [](std::string name) -> std::shared_ptr<Node> { // ★
		return std::static_pointer_cast<Node>(std::make_shared<MoveNode>(name)); // ★
	};


	lua["BT"]["Parallel"] = sol::overload(
		// パターン1: BT.Parallel("Name", "SuccessPolicy", "FailurePolicy", {child1, ...})
		[](const std::string& name, const std::string& successPolicy, const std::string& failurePolicy, sol::table children_table) -> std::shared_ptr<Node> {
		std::vector<std::shared_ptr<Node>> children;
		// Luaのテーブルから子ノードを取り出してベクターに詰める
		for (const auto& kv : children_table) {
			children.push_back(kv.second.as<std::shared_ptr<Node>>());
		}
		// ParallelNodeを生成し、基底クラスのポインタとして返す
		return std::static_pointer_cast<Node>(
			std::make_shared<ParallelNode>(name, successPolicy, failurePolicy, children)
		);
	},
		// パターン2: BT.Parallel("Name", "SuccessPolicy", {child1, ...})
		// 失敗ポリシーを省略した場合のパターン
		[](const std::string& name, const std::string& successPolicy, sol::table children_table) -> std::shared_ptr<Node> {
		std::vector<std::shared_ptr<Node>> children;
		for (const auto& kv : children_table) {
			children.push_back(kv.second.as<std::shared_ptr<Node>>());
		}
		// 失敗ポリシーのデフォルト値 "RequireOne" をC++側で補う
		return std::static_pointer_cast<Node>(
			std::make_shared<ParallelNode>(name, successPolicy, "RequireOne", children)
		);
	}
	);

	lua["BT"]["Failer"] = [](const std::string& name) -> std::shared_ptr<Node> {
		return std::static_pointer_cast<Node>(
			std::make_shared<FailerNode>(name)
		);
	};

	lua["BT"]["Succeeder"] = [](const std::string& name) -> std::shared_ptr<Node> {
		return std::static_pointer_cast<Node>(
			std::make_shared<SuccessNode>(name)
		);
	};

	lua["BT"]["Inverter"] = [](const std::string& name, std::shared_ptr<Node> child) -> std::shared_ptr<Node> {
		return std::static_pointer_cast<Node>(
			std::make_shared<InverterNode>(name, child)
		);
	};
}



void lua_setup()
{
	// --- Luaのセットアップ ---
	Lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);
	bind_nodes_to_lua(Lua);



}