# 詳細設計書 — 組込み開発実習

<!-- 作成者: あなたの名前 / 日付: YYYY-MM-DD / グループ: 〇-〇 -->

> **このドキュメントの目的**
> 基本設計書（basic_design.md）で「**どのような構造で作るか**」を決めました。
> この詳細設計書では「**各処理を具体的にどう実装するか**」を決めます。
> 書き終わったとき、**コードの骨格がほぼ完成している**状態を目指してください。

> [!NOTE]
> **V字モデルにおける位置づけ**
> 詳細設計書 ←→ **単体テスト**（関数・部品ごとのテスト）が対応します。
> 「この関数が正しく動くか」の確認は Section 5 の単体テスト仕様書で計画します。
> ※ 必須機能全体が動くかの「結合テスト」は基本設計書（Section 6）に記載します。

---

## 0. 基本設計書との接続確認

| 項目 | basic_design.md から転記 |
|:--|:--|
| 作品タイトル | 音量検出機 |
| 状態の種類（1-2 状態遷移から） | LEDやブザーなど出力の様子 |
| 実装する関数の数（2-2 関数一覧から） | 13 個 |
| グローバル変数の合計バイト数（2-1 SRAM確認から） | 13B |

---

【ピン定義】（basic_design.md 3-1 から転記）
  const int PIN_BUTTON    = 4;    // 電源ボタン（INPUT_PULLUP）
  const int PIN_MODE      = 5;    // ON/OFFボタン
  const int PIN_RESET     = 6;    // リセットボタン
  const int PIN_LED_BLUE  = 8;    // 青LED
  const int PIN_LED_GREEN = 9;    // 緑LED
  const int PIN_LED_RED   = 10;   // 赤LED
  const int PIN_SOUND     = A0;   // サウンドセンサ（アナログ入力）
  const int PIN_CLK       = 2;    // 7セグ CLK
  const int PIN_DIO       = 3;    // 7セグ DIO

【状態管理】（basic_design.md 1-2 の状態名から転記）
  int currentState = 0;   // 0:待機 1:動作中 2:完了 3:エラー
  const int STATE_WAIT    = 0;    // 待機中
  const int STATE_MEASURE = 1;    // 測定中
  const int STATE_NORMAL  = 2;    // 通常
  const int STATE_ALERT   = 3;    // 警告
  const int STATE_WEAK    = 4;    // 微弱

【タイマー（millis()用）】（basic_design.md 2-3 から転記）
  unsigned long lastMillis_LED    = 0;
  unsigned long lastMillis_Sensor = 0;
  unsigned long lastMillis_Button = 0;
  unsigned long lastMillis_Display = 0;

【センサー・入力値】（basic_design.md 2-1 から転記）
  int sensorValue   = 0;
  bool buttonState  = false;
  int displayValue  = 0;
  int ledState      = 0;

【その他のフラグ・カウンター】
  bool powerState = true;    // 電源ON/OFF状態
  int maxValue    = 0;       // 最大音量（追加機能）

【チャタリング防止用】
  unsigned long lastDebounceTime = 0;
  const int DEBOUNCE_DELAY = 50;
  int lastButtonReading = HIGH;
  int stableButtonState = HIGH;
---

## 2. 各関数の詳細設計

> ※ 基本設計書（2-2 関数一覧）で定義した各関数の「中身」を設計します。
> **疑似コード**（日本語＋処理の流れ）で書いてください。実際のC++コードは書かなくてOKです。

---

### `setup()` — 初期化処理

```
【処理の流れ】
1. ピンモードを設定する
   - PIN_BUTTON  → INPUT_PULLUP
   - PIN_MODE    → INPUT_PULLUP
   - PIN_RESET   → INPUT_PULLUP
   - PIN_LED_BLUE  → OUTPUT
   - PIN_LED_GREEN → OUTPUT
   - PIN_LED_RED   → OUTPUT
   - PIN_SOUND     → INPUT
   - PIN_CLK       → OUTPUT
   - PIN_DIO       → OUTPUT

2. ライブラリの初期化
   - lcd.begin(16, 2)（例: LCDディスプレイの初期化）
   - servo.attach(PIN_SERVO)（例: サーボモーターの初期化）

3. Serial.begin(9600)（デバッグ用）

4. 起動確認
   - 緑LEDを1秒間点灯して消灯
```

**↓ 自分の setup() を設計してください**
```
【処理の流れ】
1.
2.
3.
```

---

### `loop()` — メインループ

> ※ loop() は「状態ごとに何をするか」だけ書く。細かい処理は各関数に任せる。

```
【処理の流れ】

＜毎ループ実行すること＞
  - 入力を読む（readButton(), readSensor() などを呼ぶ）
  - 現在時刻を取得: now = millis()

＜currentState が 0（待機中）のとき＞
  - センサー値を監視する
  - 検知条件を満たしたら → currentState = 1

＜currentState が 1（動作中）のとき＞
  - メイン処理を行う
  - 終了条件を満たしたら → currentState = 2

＜currentState が 2（完了）のとき＞
  - 完了表示をする
  - リセットボタンが押されたら → currentState = 0

＜currentState が 3（エラー）のとき＞
  - エラー表示をする / リセットを待つ
```

**↓ 自分の loop() を設計してください**
```
【処理の流れ】

＜毎ループ実行すること＞


＜currentState が 　　 のとき＞


＜currentState が 　　 のとき＞


＜currentState が 　　 のとき＞

```

---

### `readButton()` — チャタリング処理済みのボタン状態を返す

**basic_design.md 2-2 との対応：** ボタン状態の取得

**引数：** なし

**戻り値：** bool

```
【処理の流れ】
1. digitalReadでボタンの生値を取得
2. 前回値と異なればlastDebounceTimeを更新
3. 50ms以上経過で状態確定し、buttonStateに反映
4. 状態変化時のみtrueを返す

【エラー・異常ケース】
- ボタン配線異常時は常にHIGH/LOWとなる→エラー表示
```

---

### `readSensor()` — センサー値を取得してsensorValueを更新

**basic_design.md 2-2 との対応：** センサー読出

**引数：** なし

**戻り値：** int

```
【処理の流れ】
1. analogReadで音量値を取得
2. ノイズ除去のため平均化（必要に応じて）
3. sensorValueに格納
4. 範囲外値は無視または前回値を使用

【エラー・異常ケース】
- センサー未接続時は0や異常値→エラー表示
```

---

### `updateOutput(state)` — 現在のstateに応じてLED/ブザーを制御

**basic_design.md 2-2 との対応：** 出力更新

**引数：** state（int）: 現在の状態

**戻り値：** void

```
【処理の流れ】
1. stateに応じてLED点灯/消灯、ブザー制御
2. 7セグ表示も必要に応じて更新
3. エラー時は全LED点滅など

【エラー・異常ケース】
- ピン出力異常時はエラー表示
```

---

### `doFeature1()` — 必須機能①の処理を行う

**basic_design.md 2-2 との対応：** 必須機能①

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. 音量を取得し、判定・記録
2. 必要に応じて状態遷移
```

---

### `doFeature2()` — 必須機能②の処理を行う

**basic_design.md 2-2 との対応：** 必須機能②

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. 音量を7セグに表示
2. 状態に応じて表示内容を切り替え
```

---

### `doOptional1()` — 追加機能①の処理を行う

**basic_design.md 2-2 との対応：** 追加機能①

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. 電源ON/OFF切替処理
2. 状態・出力を初期化
```

---

### `readSound()` — センサー値を取得する

**basic_design.md 2-2 との対応：** 音量を取得する

**引数：** なし

**戻り値：** int

```
【処理の流れ】
1. analogReadで音量値を取得
2. 必要に応じて平均化
3. 値を返す
```

---

### `updateDisplay()` — 7セグに表示する

**basic_design.md 2-2 との対応：** 音量を数値表示する

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. displayValueを7セグに出力
2. 状態に応じて表示内容を切り替え
```

---

### `controlLED()` — 音量に応じてLED変更

**basic_design.md 2-2 との対応：** LEDを制御する

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. sensorValueに応じてLED色を決定
2. 各LEDをON/OFF
```

---

### `togglePower()` — ON/OFF制御

**basic_design.md 2-2 との対応：** 電源切替

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. powerStateを反転
2. OFF時は全出力停止
```

---

### `holdMaxValue()` — 最大音量記録

**basic_design.md 2-2 との対応：** 最大値保持

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. 現在のsensorValueとmaxValueを比較
2. 大きければmaxValueを更新
```

---

### （関数ごとに以下のブロックをコピーして追加してください）

> ※ 基本設計書 2-2 の関数一覧に記載した関数を1つずつ設計します。

---

### `関数名()` — （役割を1行で書く）

**basic_design.md 2-2 との対応：** （基本設計書の関数一覧の説明を転記）

**引数：** `引数名`（型）: 何の値か

**戻り値：** 型（なしの場合は void）

```
【処理の流れ】
1.
2.
3.

【エラー・異常ケース】
- 異常な値が来た場合:
```

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス処理）

> ※ ボタンを使う場合は必ず設計してください。

```
【考え方】
  ボタンが押されたとき、50ms 以内の連続入力は「同じ1回の押下」として無視する。

【処理の流れ】
  1. 現在のボタン値を読む（digitalRead）
  2. 前回の状態と異なる場合、タイマーリセット
  3. 現在時刻 - lastDebounceTime を計算
  4. 経過時間が50ms以上なら状態を確定 
  5. 確定した状態のみ buttonState に反映

【必要な変数（Section 1 に追加済みか確認）】
  unsigned long lastDebounceTime  // 前回確定した時刻
  const int DEBOUNCE_DELAY = 50;  // チャタリング判定時間（ms）
  int lastButtonReading = HIGH;   // 前回の生の読み取り値
  int stableButtonState = HIGH;   // 確定したボタン状態
```

---

### 3-2. millis() を使ったタイマー管理

```
【考え方】
  delay()を使わず、複数の処理を同時に動かすために millis() を利用する。

【処理の流れ】
  1. 現在時刻 now = millis()　を取得
  2. now - lastMillis >= INTERVAL を確認
  3. 条件を満たした場合: 処理を実行
  4. lastMillis を now に更新

【自分のシステムで millis() を使う処理】
  （basic_design.md 2-3 のタイミング設計から転記して具体化する）

【センサー読み取り（100ms）】
1. now - lastMillis_Sensor ≥ 100 を確認
2. 条件が成立したら sensorValue を更新
3. lastMillis_Sensor = now

【LED制御（100ms）】
1. now - lastMillis_LED ≥ 100 を確認
2. 条件成立 → LED更新
3. lastMillis_LED = now

【7セグ表示（200ms）】
1. now - lastMillis_Display ≥ 200 を確認
2. 条件成立 → 表示更新
3. lastMillis_Display = now

【待機中の点滅（500ms）】
1. now - lastMillis_LED ≥ 500 を確認
2. 条件成立 → LED ON/OFF切替

```

---

### 3-3. その他の重要ロジック（任意）

> **【任意】** 複雑なロジックがある場合のみ記入してください。
> 例：「距離に応じたLED点灯パターン」「ゲームの衝突判定」「温度の閾値判定」

```
【処理の流れ】
1. サウンドセンサから音量値（0〜1023）を取得する
2. ノイズ除去のため、必要に応じて平均化する
3. 音量に応じてレベルを判定する

   ・0〜200    → 微弱
   ・200〜600  → 通常
   ・600以上   → 警告

4. 判定結果に応じて状態を更新する
5. 状態に応じてLEDと表示をする

【入力値と出力値の関係】

入力（sensorValue） → 出力

0〜200   → 青LED（微弱）
200〜600 → 緑LED（通常）
600〜1023→ 赤LED（警告）

```

---

## 4. デバッグ出力計画（任意）

> **【任意】** 関数設計（Section 2）と並行して記入すると効果的です。
> 「動かない」ときに何を確認すればいいかを事前に計画しておきます。
> 実装後は不要な Serial.println() を削除すること。

| No | 確認したい内容 | 挿入する関数 | Serial.println の内容例 |
|:---|:---|:---|:---|
| 1 | センサー値が正しく取れているか | `readSensor()` | `Serial.println(sensorValue);` |
| 2 | 状態遷移が正しく起きているか | `loop()` | `Serial.println(currentState);` |
| 3 | チャタリング処理が効いているか | `readButton()` | `Serial.println("btn confirmed");` |
| 4 | 音量判定 | `judgeLevel()` | `Serial.println("level:" + String(currentState));` |
| 5 | LED制御が正しく動くか | `controlLED()` | `Serial.println("LED updated");` |
| 6 | 表示値が正しく更新されてるか | `updateDisplay()` | `Serial.println(displayValue);` |
| 7 | 電源ON/OFF | `togglePower()` | `Serial.println(powerState);` |


---

## 5. 単体テスト仕様書（V字モデル：詳細設計 ↔ 単体テスト）

> ※ 各関数・部品が「単体で正しく動くか」を確認するテスト項目を設計します。
> 「実際の結果」欄は実装後に記入します。

### 5-1. 入力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | readButton() | ボタンを1回押す | true が1回だけ返る | | [ ] |
| 2 | readButton() | スイッチを素早く連打する | 1回分だけ 認識される | | [ ] |
| 3 | readSensor() | 通常の音声を入力 | 音量値が変化する | | [ ] |
| 4 | readSensor() | センサーを覆う | 低い値は安定して取得する | | [ ] |
| 5 | readSensor() | 大きい音声を入力する | 高い値が取得される | | [ ] |

### 5-2. 出力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | controlLED() | STATE_LOW（小音）を設定 | 青LEDが点滅 | | [ ] |
| 2 | controlLED() | STATE_NORMAL（適正）を設定 | 緑LEDが点灯 | | [ ] |
| 3 | controlLED() | STATE_NORMAL（大音）を設定 | 赤LEDが点灯 | | [ ] |
| 3 | updateDisplay() | displayValue=1234 | ７セグに「1234」と表示される | | [ ] |
| 3 | updateDisplay() |  | 表示を更新される | | [ ] |

### 5-3. タイミング・並行動作テスト

| No | テスト内容 | テスト手順 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | delay処理確認 | LED点滅中にボタンを押す | 即座に反応する | | [ ] |
| 2 | millis周期精度 | LED点滅を測定 | 500ms周期通りに点滅 | | [ ] |
| 3 | 同時処理確認 | 音入力+ボタン操作 | 両方同時に処理させる | | [ ] |
| 4 | 表示更新確認 | 音を変化させる | 数値を遅延なく変わる | | [ ] |

---

## 6. AIレビュー記録

> グループレビューの前に必ず実施してください。

### Q1: 実装上の問題確認

> 「この詳細設計書に書いた関数と処理フローをもとに Arduino でコードを書きます。バグになりやすい箇所・処理の抜け・型の問題はありますか？」

**AIの回答（要約）：**

**対応した内容：**


### Q2: 単体テスト仕様の確認

> 「Section 5 の単体テスト仕様書で、各関数の動作が正しく検証できていますか？テストが不足している項目や、境界値テストが必要な箇所を教えてください。」

**AIの回答（要約）：**

**対応した内容：**


### Q1: 実装上の問題確認

**AIの回答（要約）：**
- チャタリング処理やタイマー管理（millis使用）は、変数の型（unsigned long）や比較条件のミスに注意。
- ピンモード設定やピン番号の誤り、INPUT/OUTPUTの指定ミスに注意。
- センサー値やボタン値の範囲外チェックが抜けていると誤動作の原因になる。
- グローバル変数の初期化忘れや、状態遷移条件の漏れに注意。
- LEDや表示の制御で、複数の出力が競合しないように設計する必要がある。
- 関数の戻り値や引数の型（int, bool, voidなど）が設計通りか確認すること。
- 割り込みや非同期処理を使う場合は、変数の競合や排他制御に注意。
- デバッグ用Serial出力は、不要になったら削除すること。

**対応した内容：**
- 設計書内で型や初期化、状態遷移、エラー処理の注意点を明記。
- テスト仕様書で異常系や境界値もカバーするようにした。

---

### Q2: 単体テスト仕様の確認

**AIの回答（要約）：**
- 入力値の境界値（例：センサー値の最小・最大、ボタンの連打）テストが重要。
- 状態遷移やエラー時の動作確認も必要。
- 出力（LED・表示）の正しさや、タイミング精度もテスト項目に含めるべき。
- テストケースが不足している場合は追加する。

**対応した内容：**
- テスト仕様書に境界値・異常系・タイミング・並行動作のテストを追加。
- 状態遷移や出力の確認も網羅するようにした。

## 7. グループレビュー記録

### 7-1. 指摘一覧

| No | 指摘内容 | 指摘者 | 対応 |
|:---|:---|:---|:---|
| 1 |  |  |  |
| 2 |  |  |  |
| 3 |  |  |  |

### 7-2. レビューを受けて変更した点

-
-

---

*初版: YYYY-MM-DD / AIレビュー: YYYY-MM-DD / グループレビュー後更新: YYYY-MM-DD*
