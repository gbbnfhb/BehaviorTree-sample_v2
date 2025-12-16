raylibでBehaviorTreeのサンプル  
mainはc++で回して、ノードはluaでという感じ。  
バインダーにsol3使ってます  
今回全てAI製とはいかなかったけど9割AIどす。
というか上手く誘導してあげないと、とんでもない物書き始めるし。  
  
v2は大幅に機能追加。セレクター、リピーター、ウエイトを追加してほぼ普通のビヘイビアツリーに。  
前回はリーフだけluaだったのが今度はビヘイビアツリーほぼ全体がluaで書けるようになりました。  
~~~  
[Repeater] 'Infinite Loop' (全体を無限に繰り返す)
└── [Selector] 'Root (Reactive)' (リアクティブ・セレクター：常に優先度が高いものをチェック)
    ├── [Sequence] 'Chase Sequence' (優先度1: 追跡行動)
    │   ├── [IsEnemyNearNode] 'Is Enemy Near?' (C++: 敵が150px以内にいるか？)
    │   ├── [SetTargetToOpponentNode] 'Set Target to Enemy' (C++: ターゲットを敵の位置に設定)
    │   └── [LuaNode] 'Chase Enemy (Lua)' (Lua: ターゲットに向かって移動)
    │
    └── [Sequence] 'Wander & Wait Cycle' (優先度2: 徘徊行動)
        ├── [Repeater] 'Repeat 3 times' (3回繰り返す)
        │   └── [Sequence] 'Wander Once' (1回の徘徊)
        │       ├── [SetRandomTargetNode] 'Set Random Target' (C++: ランダムな目標地点を設定)
        │       └── [LuaNode] 'Move to Target (Lua)' (Lua: ターゲットに向かって移動)
        │
        └── [WaitNode] 'Wait 2s' (C++: 2秒間待機)
~~~  

<img width="650" height="380" alt="無題" src="https://github.com/user-attachments/assets/aeb5f320-01b5-4110-8735-116bf45d1ff9" />
