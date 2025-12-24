#pragma once

// --- BlackboardCondition.h ---
#include "DecoratorNode.h" // Decoratorの基底クラスがあると仮定
#include "Blackboard.h"
#include <functional>
#include <any>

// 比較演算子の種類をenumで定義
enum class ComparisonOperator {
	EQUAL,
	NOT_EQUAL,
	LESS_THAN,
	LESS_OR_EQUAL,
	GREATER_THAN,
	GREATER_OR_EQUAL
};

class BlackboardCondition : public DecoratorNode {
private:
	std::string key;
	ComparisonOperator op;
	std::any expected_value; // 比較対象の値も任意の型で保持

public:
	BlackboardCondition(
		const std::string& name,
		std::string blackboard_key,
		ComparisonOperator comparison_op,
		std::any value,
		std::shared_ptr<Node> child
	) : DecoratorNode(name, std::move(child)),
		key(std::move(blackboard_key)),
		op(comparison_op),
		expected_value(std::move(value)) {}

protected:
	// Decoratorの条件チェック部分
	bool checkCondition(Agent& agent) override {
		// ブラックボードにキーが存在しない場合は、即座に失敗
		if (!agent.getBlackboard()->has(key)) {
			return false;
		}

		// ブラックボードから現在の値を取得
		std::any current_value = agent.getBlackboard()->get_any(key); // get_anyは後で追加

		// 型が違う場合は比較できないので失敗
		if (current_value.type() != expected_value.type()) {
			return false;
		}

		// 型に応じて比較処理を分岐
		// bool型の場合
		if (current_value.type() == typeid(bool)) {
			return compare(std::any_cast<bool>(current_value), std::any_cast<bool>(expected_value));
		}
		// int型の場合
		if (current_value.type() == typeid(int)) {
			return compare(std::any_cast<int>(current_value), std::any_cast<int>(expected_value));
		}
		// float型の場合
		if (current_value.type() == typeid(float)) {
			return compare(std::any_cast<float>(current_value), std::any_cast<float>(expected_value));
		}
		// ... 他の型（Vector2など）も必要に応じて追加 ...

		// 対応していない型の比較は失敗とする
		return false;
	}

private:
	// 実際の比較を行うテンプレート関数
	template<typename T>
	bool compare(T current, T expected) {
		switch (op) {
		case ComparisonOperator::EQUAL:           return current == expected;
		case ComparisonOperator::NOT_EQUAL:         return current != expected;
		case ComparisonOperator::LESS_THAN:         return current < expected;
		case ComparisonOperator::LESS_OR_EQUAL:     return current <= expected;
		case ComparisonOperator::GREATER_THAN:      return current > expected;
		case ComparisonOperator::GREATER_OR_EQUAL:  return current >= expected;
		}
		return false;
	}
};