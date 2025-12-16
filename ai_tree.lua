-- ai_tree.lua
-- =================================================================
-- このAIで使う、カスタム行動ノードをLuaで定義する
-- =================================================================

-- ターゲットに向かって移動するロジック
-- この関数は、C++のMoveNode::tickと全く同じ役割を果たす
local function MoveToTarget(agent, opponent)
    -- Vector2の計算には、別途Luaでライブラリを用意するか、
    -- C++の関数をバインドする必要があるが、ここでは簡易的に実装
    local dx = agent.targetPosition.x - agent.position.x
    local dy = agent.targetPosition.y - agent.position.y
    local dist = math.sqrt(dx*dx + dy*dy)

    if dist < 2.0 then
        return "SUCCESS" -- C++側に文字列で状態を返す
    end

    -- 簡易的な移動処理（フレーム時間はC++から渡す必要があるが、ここでは省略）
    -- 実際にはGetFrameTime()をC++からグローバル変数として渡すなどする
    local frameTime = 1.0 / 60.0 
    agent.position.x = agent.position.x + (dx / dist) * agent.speed * frameTime
    agent.position.y = agent.position.y + (dy / dist) * agent.speed * frameTime

    return "RUNNING"
end




-- このスクリプトは、AIのビヘイビアツリー構造を定義し、それを返す

-- BT.Selector や BT.Sequence は、C++側で登録した関数
local tree = BT.Repeater("Infinite Loop", -1,
    BT.RSelector("Root (Reactive)", {
        -- 優先度1: 追跡行動
        BT.Sequence("Chase Sequence", {
            BT.IsEnemyNear("Is Enemy Near?", 150.0),
            BT.SetTargetToOpponent("Set Target to Enemy"),
--            BT.Move("Chase Enemy")
			BT.LuaNode("Chase Enemy (Lua)", MoveToTarget)
        }),

        -- 優先度2: 普段の徘徊サイクル
        BT.Sequence("Wander & Wait Cycle", {
            BT.Repeater("Repeat 3 times", 3,
                BT.Sequence("Wander Once", {
                    BT.SetRandomTarget("Set Random Target"),
--                    BT.Move("Move to Target")
					BT.LuaNode("Chase Enemy (Lua)", MoveToTarget)
         })
            ),
            BT.Wait("Wait 2s", 2.0)
        })
    })
)

-- 構築したツリーを返す
return tree