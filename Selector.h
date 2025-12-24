#pragma once
#include "CompositeNode.h"
#include "Service.h"

// =================================================================================================
// 2. 具体的なBTノードの実装
// =================================================================================================

// Selector: 子を順番に実行し、一つでもSUCCESSかRUNNINGになれば、そこで止める
/*
これが、私たちが今まで作ってきたSelectorです。
一度、実行中の子（RUNNINGを返す子）を見つけたら、その子の処理が終わる（ SUCCESSかFAILUREを返す）まで、
他のブランチ（より優先度の高いブランチ） を一切見ません。 
「徘徊中に敵が近づいても無視する」のは、 この性質が原因です。 
*/
class Selector : public CompositeNode {
public:
	Selector(std::string name, std::vector<std::shared_ptr<Node>> children_nodes) 
		: CompositeNode(name, std::move(children_nodes)) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {
		size_t start_index = (running_child_index != -1) ? running_child_index : 0;

		for (size_t i = start_index; i < children.size(); ++i) {
			NodeStatus child_status = children[i]->tick(agent, opponent);

			if (child_status != NodeStatus::FAILURE) {
				if (child_status == NodeStatus::RUNNING) {
					running_child_index = i;
				} else {
					running_child_index = -1;
				}
				return status = child_status;
			}
		}
		running_child_index = -1;
		return status = NodeStatus::FAILURE;
	}
	~Selector() override{
	}

};

//リアクティブ・セレクターの実装
/*
こちらも、実行中の子を見つけたら、その処理を継続します。 
しかし、毎フレーム、必ず自分より優先度の高い（左側にある） ブランチの条件をチェックします。 
もし、実行中だったブランチよりも優先度の高いブランチが実行可能になった場合、 
現在実行中のブランチを即座に強制中断（Abort） し、 新しく見つかった高優先度のブランチに処理を切り替えます。
この「リアクティブ・セレクター」こそが、 まさに「 シーケンスを途中で放棄する」 
という概念を直接的に実装したものです。 
*/
class RSelector : public CompositeNode {
protected:


public:
	RSelector(std::string name, std::vector<std::shared_ptr<Node>> children_nodes) 
		: CompositeNode(name, std::move(children_nodes)) {}

	NodeStatus tick(Agent& agent, const Agent& opponent) override {

		// このSequenceノードがtickされるたびに、アタッチされたサービスをtickする
		tickServices(agent); 

		// 毎フレーム、必ず最初の子からチェックする
		for (size_t i = 0; i < children.size(); ++i) {
			NodeStatus child_status = children[i]->tick(agent, opponent);

			// もし、その子が FAILURE でなければ (SUCCESS または RUNNING なら)
			if (child_status != NodeStatus::FAILURE) {

				// ★★★ ここからがリアクティブ処理の核心 ★★★
				// もし、現在実行中の子がいて、それが今チェックした子と違う場合
				// (つまり、より優先度の高いタスクが割り込んできた場合)
				if (running_child_index != -1 && running_child_index != i) {
					// 古いタスク（今まで実行中だった子）を強制的に中断（リセット）する
					children[running_child_index]->reset();
				}
				// ★★★ ここまでがリアクティブ処理の核心 ★★★

				// 新しく実行する子（または継続して実行する子）のインデックスを記憶
				running_child_index = i;

				// その子のステータスを、自分自身のステータスとして返す
				return status = child_status;
			}
		}

		// 全ての子が FAILURE を返した場合
		running_child_index = -1;
		return status = NodeStatus::FAILURE;
	}

	~RSelector() override{
	}

};
