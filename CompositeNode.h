#pragma once
#include "node.h"
#include "Service.h"

/*
services: このノードにアタッチされたサービスのリスト。 
addService(): 外部からサービスを追加するためのインターフェース。 
tickServices(): このノードが持つサービスをすべてチェックし、 実行すべきものがあれば実行するメソッド。 
*/


// 複数の子を持つノード (Selector, Sequence) の親
class CompositeNode : virtual public Node {
protected:

	std::vector<std::shared_ptr<Node>> children;
	int running_child_index = -1; // 実行中の子を記憶

	std::vector<std::shared_ptr<Service>> services;

public:
	virtual ~CompositeNode() = default;
	CompositeNode(std::string name, std::vector<std::shared_ptr<Node>> children_nodes) 
		: Node(name), children(std::move(children_nodes)) {}

	void reset() override {
		Node::reset();
		running_child_index = -1;
		for (const auto& child : children) {
			child->reset();
		}
	}

	std::vector<std::shared_ptr<Node>> getChildren() const override {
		return children;
	}

	void addService(std::shared_ptr<Service> service) override{
		if (service) {
			services.push_back(std::move(service));
		}
	}

	void tickServices(Agent& agent) {

		for ( auto& service : services) {
			if (service->shouldActivate()) {
				service->onTick(agent);
			}
		}

	}
};
