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


-- プレイヤーが視界内にいるかチェックし、ブラックボードを更新するサービス
local function CheckPlayerVisibility(agent)
    -- ブラックボードからopponentの情報を取得する
    -- C++側でポインタをバインドしていると仮定
    --local opponent = agent.blackboard:GetUserData("Opponent") 
	
    -- ここでは簡易的に、相手(opponent)との距離で代用
--    local dx = agent.position.x - opponent.position.x
--    local dy = agent.position.y - opponent.position.y
    local dx = agent.targetPosition.x - agent.position.x
    local dy = agent.targetPosition.y - agent.position.y
    local dist = math.sqrt(dx*dx + dy*dy)
    
    local player_is_visible = (dist < 150.0)
    
    -- ブラックボードに結果を書き込む
    agent.blackboard:SetBool("IsEnemyNear", player_is_visible)
    
    -- デバッグ出力
    --print("Service: IsEnemyNear = " .. tostring(player_is_visible))
end



---------------------------------------------------------------------
-- AIの構造定義
---------------------------------------------------------------------

-- [状態A] 戦闘行動の定義
-- 敵を発見したら、このシーケンスが実行される
local combat_behavior = BT.Sequence("Combat Sequence", {
    BT.SetTargetToOpponent("Set Target to Enemy"),
    BT.LuaNode("Chase Enemy (Lua)", MoveToTarget)
    -- 今後、ここに「攻撃する」「距離を取る」などのノードを追加していく
})


-- [状態B] 平時行動の定義
-- 敵がいない間、このシーケンスが実行される
-- この行動は一度始まったら、敵に発見されるまで中断されるべきではない
local peacetime_behavior = BT.Sequence("Peacetime Cycle", {
    BT.Repeater("Repeat 3 times", 3,
        BT.Sequence("Wander Once", {
            BT.SetRandomTarget("Set Random Target"),
            BT.LuaNode("Move to Random Target (Lua)", MoveToTarget)
        })
    ),
    BT.Wait("Wait 2s", 2.0)
})


-- [最終的な脳] 状態Aと状態Bを切り替える意思決定部分
local tree = BT.Repeater("Infinite Loop", -1,
    -- RSelectorを使い、常に敵がいないか最優先でチェックする
    BT.RSelector("Brain: Combat or Peacetime?", {
    
        -- 優先度1: 戦闘状態に入るか？
--[[
        BT.Sequence("Check for Combat Condition", {
            BT.IsEnemyNear("Is Enemy Near?", 150.0), -- 敵が近くにいるか？
            combat_behavior -- Yes -> 戦闘行動を実行
        }),
]]--
        BT.IsTrue("IsEnemyNear", 
            combat_behavior 
        ),


        -- 優先度2: 平時状態
        -- RSelectorの性質で毎フレームリセットされるのを防ぐため、
        -- Selectorでラップする。これにより、peacetime_behaviorが
        -- 完了するまで、このブランチは中断されなくなる。
        BT.Selector("Peacetime Wrapper", {
            peacetime_behavior
        })
    })

    -- ★★★ ここが新しい部分 ★★★
    -- メソッドチェーンで、RSelectorノードにサービスを追加する
    :AddService(
        -- 0.2秒間隔で CheckPlayerVisibility 関数を実行するサービスを定義
        BT.Service("Enemy Search Service", 2.0, CheckPlayerVisibility)
    )
)

-- 構築したツリーを返す
return tree
