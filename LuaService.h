#pragma once
#include "Service.h"
#include "sol3/sol.hpp" // Luaバインディングライブラリのヘッダ

// Lua関数を保持し、実行するサービス
class LuaService : public Service {
private:
	// LuaServiceは独自にnameを持つ必要があるかもしれない
	std::string service_name; 
	sol::protected_function lua_function;


public:
	LuaService(
		float interval,
		sol::protected_function func
	)
		// 親クラス(Service)には interval だけを渡す
		: Service(interval), 
		lua_function(std::move(func)) 
	{
	}

/*
	// デバッグ等で名前が必要な場合、オーバーライドする
	std::string getName() const override {
		return service_name;
	}
*/
	void onTick(Agent& agent) override {
		// (この部分のロジックは変更なし)
		if (!lua_function.valid()) return;

		auto result = lua_function(agent);
		if (!result.valid()) {
			sol::error err = result;
			std::cerr << "Lua Service Error: " << err.what() << std::endl;
		}
	}
};