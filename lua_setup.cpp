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
	};}

void lua_setup()
{
	// --- Luaのセットアップ ---
	Lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);
	bind_nodes_to_lua(Lua);



}