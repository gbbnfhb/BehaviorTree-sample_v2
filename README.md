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

~~~
ビヘイビアツリーノード一覧 (Behavior Tree Nodes)  
このプロジェクトで使用されているビヘイビアツリーの各ノードについての解説です。   
  
■コンポジットノード (Composite Nodes)  
複数の子ノードを持ち、それらを特定のルールに従って実行するノードです。
  
Selector  
 子ノードを優先度順（リストの上から下へ） に評価します。  
 SUCCESSまたはRUNNINGを返す子を最初に見つけると、そのステータスを返します。 全ての子がFAILUREを返した場合のみ、  
 自身もFAILUREを返します。 Selectorは途中で変化しません  

RSelector  
 リアクティブ・セレクター (Reactive Selector) 子ノードを優先度順（リストの上から下へ） に評価します。  
 SUCCESSまたはRUNNINGを返す子を最初に見つけると、そのステータスを返します。 全ての子がFAILUREを返した場合のみ、  
 自身もFAILUREを返します。 特徴（リアクティブ性）: 毎フレーム、現在実行中のノードよりも優先度の高い（下にある）  
 ノードが実行可能になったかを確認します。 もし実行可能になった場合、 現在実行中のタスクを即座に中断（Abort）し、  
 より優先度の高いタスクに切り替えます。 これにより、 徘徊中に敵が近づいた場合など、 状況の変化に即座に反応できます。   
    
Sequence  
 子ノードを順番（リストの上から下へ）に実行します。  途中で一つでも子ノードがFAILUREまたはRUNNINGを返すと、  
 そこで実行を中断し、そのステータスを自身の結果として返します。 全ての子ノードがSUCCESSを返した場合にのみ、  
 自身もSUCCESSを返します。  
  
Parallel  
 複数の子ノードを同時に実行します。 ポリシーとして、いずれか一つの子でもFAILUREを返した場合、  
 他の全ての子の実行を中断し、 自身もFAILUREを返します。  
 得意なこと: 「AをしながらBもする」という、並行処理を記述すること。「移動しながら攻撃する」  
 「 歩きながら周囲をキョロキョロ見回す」 「 複数の脱出条件を同時に監視する」  
 など、 一つの行動の中身を豊かにするために使います。  
 性質: 複数の子を同時に動かすため、AIの挙動に深みと複雑さをもたらします。     
   
■デコレイターノードDecorator Nodes  
 単一の子ノードを持ち、その子ノードの挙動を修飾したり、 結果を加工したりします。  
  
Repeater    
 子ノードを指定された回数、または無限に繰り返し実行します。   
 limit: 繰り返し回数を指定します。-1を指定すると無限ループになります。  
 子ノードがSUCCESSを返すたびにカウンターが1つ進み、規定回数に達するまでRUNNINGを返します。  
 規定回数に達した時点でSUCCESSを返します。  

Failer  
 子ノードを持たず、常にFAILUREを返すノード。  
  
Succeeder  
 子ノードを持たず、常にSUCCESSを返すノード。  
  
Inverter   
 子ノードの実行結果を反転させます。  
 子がSUCCESSを返せばFAILUREを返す。  
 子がFAILUREを返せばSUCCESSを返す。  
 子がRUNNINGを返した場合は、そのままRUNNINGを返す。 「敵がいない場合」のような条件を表現するのに便利です。    

■葉ノード (Leaf Nodes)  
具体的な行動や条件判断を行う、ツリーの末端ノードです。 
    
WaitNode  
 指定された時間、待機します。 待機中はRUNNINGを返し、指定時間が経過するとSUCCESSを返します。  
 duration: 待機する時間（秒）。  
  
IsEnemyNearNode  
 エージェントと敵対キャラクターとの距離を計算し、 指定された距離内かどうかを判断します。 
 distance: この距離内に敵がいればSUCCESS、いなければFAILUREを返します。  
  
SetRandomTargetNode  
 エージェントの目標地点（targetPosition）を、あらかじめ定義された範囲内のランダムな座標に設定します。  
 常にSUCCESSを返します。徘徊行動の実現に使用します。   
  
SetTargetToOpponentNode  
 エージェントの目標地点（targetPosition）を、現在の敵の位置（opponent.position）に設定します。  
 常にSUCCESSを返します。追跡行動の実現に使用します。 
  
MoveNode  
 エージェントを現在の目標地点（agent.targetPosition）に向かって移動させます。  
 目標地点に到達するまではRUNNINGを返し、到達したらSUCCESSを返します。  
 このノード自体は目標を設定せず、 移動処理に専念します。   
  
LuaNode  
 汎用Luaスクリプト実行ノード このノードのロジックはC++では実装されておらず、  
 コンストラクタで渡されたLua関数によって定義されます。  tickが呼ばれると、
 内部に保持しているLua関数を実行し、    
 その関数が返した文字列（ "  SUCCESS" ,  "FAILURE", "RUNNING"）を自身のステータスとして返します。    
 これにより、 C+ + を再コンパイルすることなく、 新しい行動をLuaスクリプトだけで迅速に実装できます。   
~~~  
