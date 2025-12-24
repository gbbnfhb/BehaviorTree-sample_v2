#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <cmath>

#define SOL_ALL_SAFETIES_ON 1
#include "sol3/sol.hpp"

#include "node.h"
#include "lua_node.h"
#include "treeView.h"

extern void lua_setup();


// =================================================================================================
// 4. メインの処理
// =================================================================================================

int main(void) {
	const int screenWidth = 1280;
	const int screenHeight = 720;
	const int GUI_WIDTH = 250;

	lua_setup();
	InitWindow(screenWidth, screenHeight, "Behavior Tree Demo");
	SetTargetFPS(60);

	// --- エージェントと敵の準備 ---
	Agent agent;
	Agent enemy;
	enemy.position = { -100, -100 }; // 最初は画面外

	// --- ビヘイビアツリーの構築 ---

	sol::protected_function_result result = Lua.script_file("ai_tree.lua");

	std::shared_ptr<Node> behaviorTreeRoot;
	if (result.valid()) {
		// スクリプトの実行に成功
		behaviorTreeRoot = result.get<std::shared_ptr<Node>>();
		std::cout << "Successfully loaded behavior tree from ai_tree.lua" << std::endl;
	} else {
		// スクリプトの実行に失敗（文法エラーなど）
		sol::error err = result;
		std::cerr << "Failed to load behavior tree from ai_tree.lua: " << err.what() << std::endl;
		// エラー処理：とりあえずダミーのノードをルートにしておくなど
		behaviorTreeRoot = std::make_shared<WaitNode>("Error Dummy", 999.0f);
	}


	// --- GUIツリーの初期化 ---
	TreeNode guiRoot(behaviorTreeRoot);
	InitializeGuiTree(guiRoot, behaviorTreeRoot);

	// --- メインループ ---
	while (!WindowShouldClose()) {
		// --- 更新処理 ---
		// 敵をマウスで動かす
		enemy.position = GetMousePosition();

		// ビヘイビアツリーを実行
		behaviorTreeRoot->tick(agent, enemy);

		// GUIツリーの状態を更新
		UpdateGuiTree(guiRoot);

		// --- 描画処理 ---
		BeginDrawing();
		ClearBackground(DARKGRAY);

		// エージェントと敵を描画
		DrawCircleV(agent.position, 15, BLUE);
		DrawCircleV(agent.targetPosition, 5, SKYBLUE);
		DrawLineV(agent.position, agent.targetPosition, SKYBLUE);
		DrawCircleV(enemy.position, 15, MAROON);
		DrawCircleLines(enemy.position.x, enemy.position.y, 150, RED); // 敵の検知範囲

		// GUIの背景
		DrawRectangle(800, 0, screenWidth - 800, screenHeight, Fade(LIGHTGRAY, 0.8f));
		DrawLine(800, 0, 800, screenHeight, BLACK);

		// GUIツリーを描画
		int treeViewY = 10;
		DrawTreeView(guiRoot, treeViewY, 0);

		// リセットボタン
		if (GuiButton(Rectangle{810, screenHeight - 40, 100, 30}, "Reset BT")) {
			behaviorTreeRoot->reset();
		}

		DrawFPS(10, 10);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}