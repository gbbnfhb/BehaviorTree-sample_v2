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

#include "luaService.h"
#include "lua_node.h"
#include "BlackboardCondition.h"
#include <any>

sol::state Lua;

// 文字列を比較演算子enumに変換するヘルパー関数
ComparisonOperator StringToOp(const std::string& str_op) {
	if (str_op == "==") return ComparisonOperator::EQUAL;
	if (str_op == "!=") return ComparisonOperator::NOT_EQUAL;
	if (str_op == "<")  return ComparisonOperator::LESS_THAN;
	if (str_op == "<=") return ComparisonOperator::LESS_OR_EQUAL;
	if (str_op == ">")  return ComparisonOperator::GREATER_THAN;
	if (str_op == ">=") return ComparisonOperator::GREATER_OR_EQUAL;
	return ComparisonOperator::EQUAL; // デフォルト
}

std::any ToAny(sol::object lua_object) {
	// sol::objectが有効かどうかをチェック
	if (!lua_object.valid()) {
		return {}; // 無効な場合は空のstd::anyを返す
	}

	// Luaの型を判別し、対応するC++の型でstd::anyに格納する
	switch (lua_object.get_type()) {
	case sol::type::boolean:
	// bool型として取得し、std::anyに格納
	return lua_object.as<bool>();

	case sol::type::number:
	// Luaのnumber型は、C++では整数か浮動小数点数か判別が必要
	if (lua_object.is<int>()) {
		return lua_object.as<int>();
	} else {
		// 整数でなければ、とりあえずdouble/floatとして扱う
		return lua_object.as<float>(); 
	}

	case sol::type::string:
	// string型として取得し、std::anyに格納
	return lua_object.as<std::string>();

	case sol::type::userdata:
	// Vector2のようなカスタム型（userdata）の場合
	if (lua_object.is<Vector2>()) {
		return lua_object.as<Vector2>();
	}
	// 他のuserdataも必要に応じて追加
	break;

	default:
	// 対応していない型の場合は空のstd::anyを返す
	return {};
	}

	return {}; // switchを抜けた場合（userdataで一致しなかったなど）
}

void bind_nodes_to_lua(sol::state& lua) {
	lua.new_usertype<Vector2>("Vector2",
		"x", &Vector2::x,
		"y", &Vector2::y
	);

	lua.new_usertype<Agent>("Agent",
		"position", &Agent::position,
		"targetPosition", &Agent::targetPosition,
		"speed", &Agent::speed,
		"blackboard", &Agent::bb

	);

	lua.new_usertype<FailerNode>("Failer",
		sol::no_constructor,
		sol::base_classes, sol::bases<Node>()
	);

	lua.new_usertype<SuccessNode>("Succeeder",
                sol::no_constructor,
                sol::base_classes, sol::bases<Node>()
        );

        lua.new_usertype<DecoratorNode>("DecoratorNode",
                sol::no_constructor,
                sol::base_classes, sol::bases<Node>()
        );

        lua.new_usertype<BlackboardCondition>("BlackboardCondition",
                sol::no_constructor,
                sol::base_classes, sol::bases<DecoratorNode, Node>()
        );

		lua.new_usertype<Blackboard>("Blackboard",
			// メソッドを登録
			"SetBool", &Blackboard::SetBool
			// 他のSet/Getメソッドもここに追加
		);
	

	// --- 基本的なノードクラスをLuaに登録 ---
	// これにより、Lua側でNodeオブジェクトを扱えるようになる
		auto node_type = lua.new_usertype<Node>("Node", sol::no_constructor);

	lua.new_usertype<CompositeNode>("CompositeNode", sol::no_constructor, 	sol::base_classes, sol::bases<Node>());
	lua.new_usertype<DecoratorNode>("DecoratorNode",	sol::base_classes, sol::bases<Node>());

	lua.new_usertype<Selector>("Selector", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<RSelector>("RSelector", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<Sequence>("Sequence", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<Repeater>("Repeater", sol::no_constructor, sol::base_classes, sol::bases<Node, DecoratorNode>());
	lua.new_usertype<WaitNode>("WaitNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<IsEnemyNearNode>("IsEnemyNearNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<SetTargetToOpponentNode>("SetTargetToOpponentNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<SetRandomTargetNode>("SetRandomTargetNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<MoveNode>("MoveNode", sol::no_constructor, sol::base_classes, sol::bases<Node>());
	lua.new_usertype<ParallelNode>("Parallel",sol::no_constructor, sol::base_classes, sol::bases<Node, CompositeNode>());
	lua.new_usertype<InverterNode>("Inverter",sol::no_constructor,	sol::base_classes, sol::bases<Node>());
	lua.new_usertype<BlackboardCondition>("BlackboardCondition", sol::base_classes, sol::bases<Node, DecoratorNode>());

        lua.new_usertype<Service>("Service", sol::no_constructor);
        lua.new_usertype<LuaService>("LuaService", sol::no_constructor, sol::base_classes, sol::bases<Service>());
	// Luaの "AddService" という名前のメソッドを、C++の "addServiceAndReturnSelf" に紐付ける
	node_type["AddService"] = [](std::shared_ptr<Node> self, std::shared_ptr<Service> service) -> std::shared_ptr<Node> {
                if (self) {
                        self->addServiceAndReturnSelf(service);
                }
                return self;
        };


	// --- 各ノードを生成するための「ファクトリ関数」をLuaに公開 ---
	// Luaのグローバル空間に、BTというテーブル（名前空間のようなもの）を作る
	lua["BT"] = lua.create_table();

	lua["BT"]["IsTrue"] = [](const std::string& key, std::shared_ptr<Node> child) -> std::shared_ptr<Node> {
		// 面倒な引数をC++側で肩代わりして、BlackboardConditionを生成する
		std::string name = "IsTrue: " + key;
		std::any expected_value = true;
		return std::make_shared<BlackboardCondition>(name, key, ComparisonOperator::EQUAL, expected_value, child);
	};

	lua["BT"]["IsFalse"] = [](const std::string& key, std::shared_ptr<Node> child) -> std::shared_ptr<Node> {
		std::string name = "IsFalse: " + key;
		std::any expected_value = false;
		return std::make_shared<BlackboardCondition>(name, key, ComparisonOperator::EQUAL, expected_value, child);
	};

	lua["BT"]["CompareNumber"] = [](
		const std::string& key,
		const std::string& op_str,
		float value, // Luaのnumberはfloat/doubleで受けるのが安全
		std::shared_ptr<Node> child
		) -> std::shared_ptr<Node> {
		std::string name = "Compare " + key + " " + op_str + " " + std::to_string(value);
		std::any expected_value = value;
		return std::make_shared<BlackboardCondition>(name, key, StringToOp(op_str), expected_value, child);
	};

	lua["BT"]["Service"] = [](const std::string& name, float interval, sol::protected_function func) -> std::shared_ptr<Service> {
		// Lua関数をラップしたLuaServiceオブジェクトを生成して返す
		return std::make_shared<LuaService>(interval, func);
	};


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
#if 0
	lua["BT"]["Sequence"] = [](std::string name, sol::table children_table) -> std::shared_ptr<Node> {
		std::vector<std::shared_ptr<Node>> children;
		for (const auto& kv : children_table) {
			children.push_back(kv.second.as<std::shared_ptr<Node>>());
		}
		return std::static_pointer_cast<Node>(std::make_shared<Sequence>(name, children));
	};
#else
	lua["BT"]["Sequence"] = [](const std::string& name, const sol::table& children_table) {
		std::vector<std::shared_ptr<Node>> children;
		for (const auto& pair : children_table) {
			// Luaのテーブルから子ノードを取り出してベクターに入れる
			children.push_back(pair.second.as<std::shared_ptr<Node>>());
		}
		// Sequenceオブジェクトを生成して返す
//		return std::make_shared<Sequence>(name, std::move(children));
		return std::static_pointer_cast<Node>(std::make_shared<Sequence>(name, children));
	};
#endif


	lua["BT"]["Repeater"] = [](std::string name, int limit, const std::shared_ptr<Node>& child) -> std::shared_ptr<Node> {
		return std::static_pointer_cast<Node>(std::make_shared<Repeater>(name, limit, child));
	};

	lua["BT"]["Wait"] = [](std::string name, float duration) -> std::shared_ptr<Node> {
		return std::static_pointer_cast<Node>(std::make_shared<WaitNode>(name, duration));
	};

	lua["BT"]["IsEnemyNear"] = [](std::string name, float distance) -> std::shared_ptr<Node> {
		return std::static_pointer_cast<Node>(std::make_shared<IsEnemyNearNode>(name, distance));
	};

	lua["BT"]["SetTargetToOpponent"] = [](std::string name) -> std::shared_ptr<Node> {
		return std::static_pointer_cast<Node>(std::make_shared<SetTargetToOpponentNode>(name));
	};

	lua["BT"]["SetRandomTarget"] = [](std::string name) -> std::shared_ptr<Node> { 
		return std::static_pointer_cast<Node>(std::make_shared<SetRandomTargetNode>(name));
	};

	lua["BT"]["Move"] = [](std::string name) -> std::shared_ptr<Node> {
		return std::static_pointer_cast<Node>(std::make_shared<MoveNode>(name));
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
	//lua_gc (Lua, LUA_GCSTOP,0);



}