#pragma once
#include <chrono>

class Agent; //前方宣言


/*
interval: サービスが実行される間隔（秒）。
last_activation_time: 最後に実行された時間。
onTick(): サービスの具体的な処理内容。これを派生クラスで実装します。 
shouldActivate(): 前回の実行から指定した時間が経過したかをチェックするヘルパー関数。 
*/
// サービスの基底クラス
class Service {
protected:
	std::chrono::steady_clock::time_point last_activation_time;
	std::chrono::duration<float> interval;

public:
	Service(float interval_seconds) : interval(interval_seconds) {}
	virtual ~Service() = default;

	// サービスの本体。派生クラスで実装する
	virtual void onTick(Agent& agent) = 0;

	// 実行すべきタイミングかチェックする
	bool shouldActivate() {
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::duration<float>>(now - last_activation_time).count() >= interval.count()) {
			last_activation_time = now;
			return true;
		}
		return false;
	}
};
