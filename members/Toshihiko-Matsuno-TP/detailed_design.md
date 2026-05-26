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
| 作品タイトル | 赤ん坊あやし機 |
| 状態の種類（1-2 状態遷移から） | 待機中 / センサー検知 / 計測 / 閾値超え / 音楽再生中 / LED点灯中 / ファン駆動中 / 5秒待機中 |
| 実装する関数の数（2-2 関数一覧から） | 12個 |
| グローバル変数の合計バイト数（2-1 SRAM確認から） | 未記入（実装前に算出） |

---

## 1. グローバル変数・定数の設計

> ※ 基本設計書（2-1 データ設計）をもとに、**型と初期値まで**決めます。
> ここで設計した変数は、この後の関数設計でそのまま使います。

```
【ピン定義】（basic_design.md 3-1 から転記）
  PIN_SOUND_SENSOR = 7    // Sound sensor module
  PIN_BTN_LED      = 2    // LED切替ボタン（INPUT_PULLUP）
  PIN_BTN_MUSIC    = 3    // 音楽切替ボタン（INPUT_PULLUP）
  PIN_BTN_STOP     = 4    // 強制終了ボタン（INPUT_PULLUP）
  PIN_BTN_FAN      = 5    // ファン切替ボタン（INPUT_PULLUP）
  PIN_FAN          = 8    // ファン出力（トランジスタ経由）
  PIN_BUZZER       = 9    // Passive Buzzer
  PIN_LED_GREEN    = 10   // LED（緑）
  PIN_MAX7219_DIN  = 11   // Max7219 DIN（任意）
  PIN_MAX7219_CS   = 12   // Max7219 CS（任意）
  PIN_MAX7219_CLK  = 13   // Max7219 CLK（任意）

【状態管理】（basic_design.md 1-2 の状態名から転記）
  currentState  : int = 0   // 0:待機 1:センサー検知 2:計測 3:閾値超え 4:音楽再生 5:5秒待機
  STATE_IDLE       : const int = 0
  STATE_DETECTED   : const int = 1
  STATE_MEASURING  : const int = 2
  STATE_TRIGGERED  : const int = 3
  STATE_PLAYING    : const int = 4
  STATE_COOLDOWN   : const int = 5

【タイマー（millis()用）】（basic_design.md 2-3 から転記）
  lastMillis_Button   : unsigned long = 0  // 100ms周期
  lastMillis_Sensor   : unsigned long = 0  // 100ms周期
  lastMillis_Music    : unsigned long = 0  // 10〜20ms周期
  cooldownStartMillis : unsigned long = 0  // 5秒待機開始時刻
  INTERVAL_BUTTON_MS  : const unsigned long = 100
  INTERVAL_SENSOR_MS  : const unsigned long = 100
  INTERVAL_MUSIC_MS   : const unsigned long = 20
  COOLDOWN_MS         : const unsigned long = 5000
  DEBOUNCE_DELAY_MS   : const unsigned long = 50

【センサー・入力値】（basic_design.md 2-1 から転記）
  sound_vol    : int  = 0      // 音量判定値
  stop_botan   : int  = 0      // 0:未動作 1:作動
  music_botan  : bool = true   // 音楽再生許可
  led_botan    : bool = true   // LED点灯許可
  fun_botan    : bool = true   // ファン駆動許可
  lastButtonState_STOP : bool = true

【その他のフラグ・カウンター】
  music      : int = 0   // 0:停止 1:再生中
  led        : int = 0   // 0:消灯 1:点灯中
  fun        : int = 0   // 0:停止 1:回転中
  currentTuneId : int = 0
  randomMode    : bool = true
  SOUND_THRESHOLD_ON  : const int = 80
  SOUND_THRESHOLD_OFF : const int = 75
```

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
   - PIN_LED_*   → OUTPUT
   - PIN_BUZZER  → OUTPUT

2. ライブラリの初期化（使うものだけ）
   - 例: lcd.begin(16, 2)
   - 例: servo.attach(PIN_SERVO)

3. Serial.begin(9600)（デバッグ用）

4. 起動確認（任意）: 緑LEDを1秒点灯して消灯
```

**↓ 自分の setup() を設計してください**
```
【処理の流れ】
1. 入出力ピンを初期化する
  - PIN_SOUND_SENSOR, PIN_BTN_LED, PIN_BTN_MUSIC, PIN_BTN_STOP, PIN_BTN_FAN → INPUT_PULLUP
  - PIN_FAN, PIN_BUZZER, PIN_LED_GREEN → OUTPUT
  - （任意機能を使う場合）PIN_MAX7219_DIN/CS/CLK をSPI出力として準備

2. 起動時の初期状態を設定する
  - currentState = STATE_IDLE
  - music = 0, led = 0, fun = 0
  - music_botan = true, led_botan = true, fun_botan = true
  - lastMillis_Button/Sensor/Music と cooldownStartMillis を 0 で初期化

3. 安全な初期出力を確定する
  - ファン停止、ブザー停止、LED消灯
  - Serial.begin(9600) を開始
  - 起動確認としてLEDを短く点灯してから消灯
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
- now = millis() を取得する
- 強制終了ボタンの状態を最優先で確認する
- 100msごとに各ボタン（LED/音楽/ファン切替）を読んで許可フラグを更新する
- 100msごとに音センサー値を取得し、sound_vol を更新する

＜currentState が STATE_IDLE（待機） のとき＞
- 出力は停止状態を維持する（fan/buzzer/LED OFF）
- sound_vol が SOUND_THRESHOLD_ON 以上なら currentState = STATE_DETECTED へ遷移する

＜currentState が STATE_DETECTED（センサー検知） のとき＞
- ノイズ除去のため短い確認判定を行う
- 連続して閾値以上なら currentState = STATE_MEASURING
- 閾値未満へ戻ったら currentState = STATE_IDLE

＜currentState が STATE_MEASURING（計測） のとき＞
- 閾値判定を確定する
- 確定で閾値超えなら currentState = STATE_TRIGGERED
- 不成立なら currentState = STATE_IDLE

＜currentState が STATE_TRIGGERED（閾値超え） のとき＞
- selectMusic() で再生曲を決定する（順番またはランダム）
- music_botan が true なら currentState = STATE_PLAYING
- music_botan が false の場合は音なし動作として LED/FAN のみ許可条件で実行後、cooldownStartMillis = now, currentState = STATE_COOLDOWN

＜currentState が STATE_PLAYING（再生中） のとき＞
- INTERVAL_MUSIC_MS ごとに playMusic() を進める
- led_botan が true なら controlLED() を実行（任意で ledPatternToMusic() を使用）
- fun_botan が true なら controlFan(true) を実行
- 強制終了ボタン押下、または曲終了で出力停止し、cooldownStartMillis = now, currentState = STATE_COOLDOWN

＜currentState が STATE_COOLDOWN（5秒待機） のとき＞
- すべての出力を停止したまま維持する
- now - cooldownStartMillis >= COOLDOWN_MS になったら currentState = STATE_IDLE へ戻る

＜どの状態でも共通の例外処理＞
- 強制終了ボタン押下時は即時に fan/buzzer/LED を停止し、cooldownStartMillis = now, currentState = STATE_COOLDOWN
```

---


---

### `checkSoundSensor()` — 音センサー値の取得と閾値判定

**basic_design.md 2-2 との対応：** 音センサーから値を取得し、閾値判定を行う。ノイズ除去も担当。

**引数：** なし

**戻り値：** int（現在の音量値）

```
【処理の流れ】
1. PIN_SOUND_SENSOR からアナログ値を取得し sound_vol に格納
2. 取得値がSOUND_THRESHOLD_ON以上か判定
3. ノイズ除去のため、一定回数連続で閾値超えを確認
4. 閾値未満ならカウンタをリセット
5. 判定結果を返す

【エラー・異常ケース】
- センサー値が異常（0や最大値連発）の場合はエラーとしてログ出力
```

---

### `playMusic()` — ブザーで音楽を再生する

**basic_design.md 2-2 との対応：** 曲データに従い、ブザーで音楽を再生する。再生中は状態を管理。

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. currentTuneId で指定された曲データを参照
2. INTERVAL_MUSIC_MSごとに次の音階データを出力
3. 曲が終了したらmusic=0にし、出力を停止
4. 再生中は必要に応じてLEDやファンも連動制御

【エラー・異常ケース】
- 曲データが不正・範囲外の場合は再生を中断しエラー出力
```

---

### `controlFan(on)` — ファンのON/OFF制御

**basic_design.md 2-2 との対応：** ファンの駆動・停止を制御する。安全のため強制停止も考慮。

**引数：** `on`（bool）: trueでON, falseでOFF

**戻り値：** void

```
【処理の流れ】
1. on==true なら PIN_FAN をHIGHにしてファンを回す
2. on==false なら PIN_FAN をLOWにしてファンを止める
3. 状態変化時はfun変数も更新

【エラー・異常ケース】
- ピン出力に失敗した場合はエラー出力
```

---

### `controlLED(on)` — LEDのON/OFF制御

**basic_design.md 2-2 との対応：** 緑LEDの点灯・消灯を制御。音楽や状態に連動。

**引数：** `on`（bool）: trueでON, falseでOFF

**戻り値：** void

```
【処理の流れ】
1. on==true なら PIN_LED_GREEN をHIGHにして点灯
2. on==false なら PIN_LED_GREEN をLOWにして消灯
3. 状態変化時はled変数も更新

【エラー・異常ケース】
- ピン出力に失敗した場合はエラー出力
```

---

### `ledPatternToMusic(tuneId)` — 曲ごとにLEDパターンを切替

**basic_design.md 2-2 との対応：** 曲ごとにLEDの点灯パターンを変化させる。

**引数：** `tuneId`（int）: 再生中の曲ID

**戻り値：** void

```
【処理の流れ】
1. tuneIdに応じてLED点灯パターンを決定
2. パターンに従いPIN_LED_GREENを制御
3. 必要に応じてタイマーで点滅制御

【エラー・異常ケース】
- tuneIdが未定義の場合はデフォルトパターンにフォールバック
```

---

### （関数ごとに以下のブロックをコピーして追加してください）

> ※ 基本設計書 2-2 の関数一覧に記載した関数を1つずつ設計します。
### `selectMusic()` — 再生する曲を決定

**basic_design.md 2-2 との対応：** 曲リストから再生曲を選択。ランダム/順番切替も担当。

**引数：** なし

**戻り値：** int（選択された曲ID）

```
【処理の流れ】
1. randomMode==trueなら乱数で曲IDを選択
2. falseならcurrentTuneIdをインクリメント（曲リストを順番に）
3. currentTuneIdに格納し、返す

【エラー・異常ケース】
- 曲リストが空・範囲外の場合は0番にフォールバック
```

---

### `関数名()` — （役割を1行で書く）

### `controlMax7219(pattern)` — Max7219 LEDマトリクス制御

**basic_design.md 2-2 との対応：** Max7219モジュールでLEDマトリクス表示を制御

**引数：** `pattern`（int/配列）: 表示したいパターン

**戻り値：** void

```
【処理の流れ】
1. patternに応じてMax7219へデータ送信
2. SPI通信で表示を更新
3. 必要に応じてパターン切替やアニメーション

【エラー・異常ケース】
- SPI通信エラー時は再送・エラー出力
```

---

### `readButton()` — チャタリング処理済みのボタン状態を返す

**basic_design.md 2-2 との対応：** 各種ボタン（LED/音楽/ファン/強制終了）の状態を読み、チャタリング除去後の確定値を返す

**引数：** なし

**戻り値：** bool（ボタンが押されたか）

```
【処理の流れ】
1. 各ボタンピン（PIN_BTN_LED, PIN_BTN_MUSIC, PIN_BTN_STOP, PIN_BTN_FAN）をdigitalReadで取得
2. 前回確定時刻（lastDebounceTime）からの経過時間を確認
3. DEBOUNCE_DELAY_MS未満なら無視、以上なら状態を確定
4. 状態変化時のみtrueを返す
5. lastDebounceTimeを更新

【エラー・異常ケース】
- ピン値が常時HIGH/LOWなど異常な場合はエラー出力
```

---

### `readSensor()` — センサー値を取得してsound_volを更新

**basic_design.md 2-2 との対応：** 音センサーから値を取得し、sound_volに格納する

**引数：** なし

**戻り値：** int（取得した音量値）

```
【処理の流れ】
1. PIN_SOUND_SENSORからアナログ値を取得
2. sound_volに格納
3. 取得値が仕様範囲外（例:0や最大値）なら前回値を保持
4. 取得値を返す

【エラー・異常ケース】
- センサー値が異常な場合はエラー出力
```

---

### `updateOutput(state)` — 現在のstateに応じてLED/ブザー/ファンを制御

**basic_design.md 2-2 との対応：** 状態に応じて各出力（LED/ブザー/ファン）をON/OFF制御

**引数：** `state`（int）: 現在の状態

**戻り値：** void

```
【処理の流れ】
1. stateに応じてLED/ブザー/ファンのON/OFFを決定
2. controlLED(), playMusic(), controlFan()等を呼び出し
3. 必要に応じてMax7219も制御
4. 出力状態を変化させた場合はログ出力

【エラー・異常ケース】
- ピン出力や制御関数でエラーが発生した場合はエラー出力
```

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス処理）

> ※ ボタンを使う場合は必ず設計してください。

```
【考え方】
  ボタンが押されたとき、50ms 以内の連続入力は「同じ1回の押下」として無視する。

【処理の流れ】
  1. ボタンのデジタル値を読む（digitalRead）
  2. 前回確定した時刻（lastDebounceTime）からの経過時間を計算する
  3. 経過時間 < DEBOUNCE_DELAY（例: 50ms）→ 無視する
  4. 経過時間 ≥ DEBOUNCE_DELAY → ボタンの状態として確定する
  5. lastDebounceTime を更新する

【必要な変数（Section 1 に追加済みか確認）】
  lastDebounceTime : unsigned long   // 前回確定した時刻
  DEBOUNCE_DELAY   : const int = 50  // チャタリング判定時間（ms）
```

---

### 3-2. millis() を使ったタイマー管理

```
【考え方】
  「前回実行した時刻」を記録しておき、「今の時刻 − 前回時刻 ≥ 周期」なら実行する。

【処理の流れ（例: LED点滅）】
  1. now = millis()
  2. now - lastMillis_LED >= LED_INTERVAL かどうか確認
  3. 条件を満たした場合: LEDのON/OFFを切り替え、lastMillis_LED = now
  4. 条件を満たさない場合: 何もしない（次のループで再チェック）

【自分のシステムで millis() を使う処理】
  （basic_design.md 2-3 のタイミング設計から転記して具体化する）
```

---

### 3-3. その他の重要ロジック（任意）

> **【任意】** 複雑なロジックがある場合のみ記入してください。
> 例：「距離に応じたLED点灯パターン」「ゲームの衝突判定」「温度の閾値判定」

```
【考え方：音センサーのヒステリシス判定】
  音センサーは閾値付近で値が揺れやすく、ON/OFF判定が頻繁に切り替わると誤動作や不快な出力につながる。そのため、ON判定とOFF判定で異なる閾値（ヒステリシス）を設け、安定した状態遷移を実現する。

【処理の流れ：音センサーのヒステリシス判定（ON/OFF閾値分離）】
1. 音センサー値がSOUND_THRESHOLD_ON（例:80dB）以上でON判定
2. 一度ONになった後、SOUND_THRESHOLD_OFF（例:75dB）未満でOFF判定
3. 閾値付近で値が揺れてもON/OFFが頻繁に切り替わらないようにする

【入力値と出力値の関係】
・80dB以上でON、75dB未満でOFF、それ以外は現状維持

---

【考え方：安全停止処理】
  センサーやボタンの異常、想定外の動作が発生した場合、出力を継続すると安全上のリスクや誤動作につながる。異常検知時は全ての出力を即時OFFし、復旧操作があるまで安全な待機状態を維持することで、システムの信頼性と安全性を高める。

【処理の流れ：安全停止処理（異常時の全出力停止）】
1. センサー値やボタン入力に異常が発生した場合、全ての出力（LED/ブザー/ファン/Max7219）を即時OFF
2. エラー内容をSerial出力等で記録
3. 復旧条件（例:リセットボタン押下）を満たすまで待機状態を維持

【入力値と出力値の関係】
・異常検知時は全出力OFF、復旧後に通常動作へ復帰
```
【処理の流れ：音センサーのヒステリシス判定（ON/OFF閾値分離）】
1. 音センサー値がSOUND_THRESHOLD_ON（例:80dB）以上でON判定
2. 一度ONになった後、SOUND_THRESHOLD_OFF（例:75dB）未満でOFF判定
3. 閾値付近で値が揺れてもON/OFFが頻繁に切り替わらないようにする

【入力値と出力値の関係】
・80dB以上でON、75dB未満でOFF、それ以外は現状維持

---

【処理の流れ：安全停止処理（異常時の全出力停止）】
1. センサー値やボタン入力に異常が発生した場合、全ての出力（LED/ブザー/ファン/Max7219）を即時OFF
2. エラー内容をSerial出力等で記録
3. 復旧条件（例:リセットボタン押下）を満たすまで待機状態を維持

【入力値と出力値の関係】
・異常検知時は全出力OFF、復旧後に通常動作へ復帰
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
| 4 |  |  |  |

---

## 5. 単体テスト仕様書（V字モデル：詳細設計 ↔ 単体テスト）
> 「実際の結果」欄は実装後に記入します。

> ※ 各関数・部品が「単体で正しく動くか」を確認するテスト項目を設計します。
> 「実際の結果」欄は実装後に記入します。

### 5-1. 入力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | readButton() | 各ボタンを1回押す | true が返る | | [ ] |
| 2 | readButton() | ボタンを素早く2回押す | 1回分だけ true になる（チャタリング除去） | | [ ] |
| 3 | readSensor() | センサーを正常範囲で使う | 仕様範囲内の値が返る | | [ ] |
| 4 | readSensor() | センサーを遮蔽・範囲外に向ける | 異常値は無視・前回値を保持 | | [ ] |
| 5 | checkSoundSensor() | 80dB以上の音を入力 | 閾値超えでON判定 | | [ ] |
| 6 | checkSoundSensor() | 75dB未満の音を入力 | OFF判定（ヒステリシス動作） | | [ ] |
| 7 | selectMusic() | ランダム/順番切替で複数回呼ぶ | 曲IDが正しく切り替わる | | [ ] |

### 5-2. 出力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | updateOutput(STATE_IDLE) | state=STATE_IDLEを渡す | 全出力OFF | | [ ] |
| 2 | updateOutput(STATE_PLAYING) | state=STATE_PLAYINGを渡す | 音楽再生・LED点灯・ファン回転 | | [ ] |
| 3 | controlLED(true/false) | on/offを切り替え | LEDが点灯/消灯 | | [ ] |
| 4 | controlFan(true/false) | on/offを切り替え | ファンが回転/停止 | | [ ] |
| 5 | playMusic() | 曲データを与えて再生 | 曲が正しく再生される | | [ ] |
| 6 | ledPatternToMusic() | 曲IDごとに呼ぶ | LEDパターンが切り替わる | | [ ] |
| 7 | controlMax7219() | パターンを与えて表示 | LEDマトリクスが正しく表示 | | [ ] |

### 5-3. タイミング・並行動作テスト

| No | テスト内容 | テスト手順 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | delay()による処理停止がないか | LED点滅中にボタンを押す | ボタン入力が無視されない | | [ ] |
| 2 | millis()タイマーの周期精度 | 点滅や音楽再生をストップウォッチで確認 | 設計した周期通りに動作 | | [ ] |
| 3 | 並行動作 | 音楽再生中にLED/ファン/ボタン操作 | すべての処理が並行して動作 | | [ ] |

### 5-4. 異常系・安全系テスト

| No | テスト内容 | テスト手順 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | チャタリング除去 | ボタンを連打 | 1回分だけ反応 | | [ ] |
| 2 | センサー異常値 | センサーを外す/短絡 | 出力が停止・エラー記録 | | [ ] |
| 3 | 強制終了ボタン | 再生中に強制終了ボタン押下 | 全出力が即時OFF・待機状態へ | | [ ] |
| 4 | 安全停止処理 | 異常発生時に復旧操作 | 復旧後に通常動作へ戻る | | [ ] |

---

## 6. AIレビュー記録

> グループレビューの前に必ず実施してください。

### Q1: 実装上の問題確認

> 「この詳細設計書に書いた関数と処理フローをもとに Arduino でコードを書きます。バグになりやすい箇所・処理の抜け・型の問題はありますか？」

**AIの回答（要約）：**
- 各関数は単一責任で分割されており、状態遷移・出力制御・異常系も明確に設計されている。
- ピン割り当てやグローバル変数の型・初期値も明記されており、型の不整合リスクは低い。
- 音センサーのヒステリシス判定やチャタリング除去、安全停止処理など、誤動作・安全性への配慮も十分。
- millis()による非ブロッキング設計で、並行動作や入力取りこぼしも考慮されている。
- 懸念点としては、センサー値の異常時や曲データの不正時のエラー処理、SPI通信エラー時の再送など、異常系の実装漏れに注意。

**対応した内容：**
- 各関数の役割・引数・戻り値・異常系を明記し、単一責任・安全設計を徹底。
- 状態遷移・出力制御・異常時の全出力OFF・復旧処理を明文化。
- 型・初期値・ピン割り当てをSection 1で明示し、型の不整合を防止。
- ヒステリシス・チャタリング・安全停止など、誤動作防止ロジックをSection 3で設計。
- エラー時のログ出力や復旧条件も明記。

---

### Q2: 単体テスト仕様の確認

> 「Section 5 の単体テスト仕様書で、各関数の動作が正しく検証できていますか？テストが不足している項目や、境界値テストが必要な箇所を教えてください。」

**AIの回答（要約）：**
- 入力系・出力系・タイミング・異常系を網羅したテスト項目が設計されている。
- チャタリング・ヒステリシス・強制終了・安全停止など、境界値や異常系もテスト対象に含まれている。
- 並行動作や周期精度、異常時の復旧など、実装上のリスク箇所もテストでカバー。
- 追加で考慮すべきは、曲データの不正・SPI通信エラー・ピン断線など、ハード依存の異常系テスト。

**対応した内容：**
- Section 5で各関数・ロジックごとに単体テスト項目を具体化。
- 境界値（閾値付近、チャタリング、タイマー周期）や異常系（センサー異常、通信エラー、強制終了）も網羅。
- 実装後は「実際の結果」欄にテスト結果を記録し、合否を明確化。

---

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


8-1「星の奏でる歌」の仮コード
const int buzzerPin = 8;

// ===== テンポ =====
int tempo = 75;
float beat = 60000.0 / tempo;

// ===== 音階（原曲キー F#系）=====
#define F_SHARP4 370
#define G_SHARP4 415
#define A_SHARP4 466
#define B4 494
#define C_SHARP5 554
#define D_SHARP5 622
#define F_SHARP5 740

#define E5 659
#define F5 698
#define G5 784


#define G_SHARP5 831
#define A_SHARP5 932

// ===== メロディ（1番 / 原曲キー変換済）=====
int melody[] = {

  // ♪ 探し物 ひとつ
  A_SHARP4,A_SHARP4,B4,C_SHARP5, C_SHARP5,B4,A_SHARP4,G_SHARP4,

  // ♪ 星の 笑う声
  A_SHARP4,B4,C_SHARP5,D_SHARP5, C_SHARP5,B4,A_SHARP4,0,

  // ♪ 風に またたいて
  C_SHARP5,D_SHARP5,F_SHARP5,E5, D_SHARP5,C_SHARP5,A_SHARP4,0,

  // ♪ 手を伸ばせば
  A_SHARP4,B4,C_SHARP5,D_SHARP5, C_SHARP5,A_SHARP4,G_SHARP4,0,

  // ♪ 掴めるよ
  A_SHARP4,C_SHARP5,D_SHARP5,C_SHARP5, A_SHARP4,0,

  0,0,

  // ♪ 瞳閉じれば
  D_SHARP5,C_SHARP5,B4,A_SHARP4, G_SHARP4,A_SHARP4,B4,C_SHARP5,

  // ♪ きっと見つかる
  D_SHARP5,F5,G5,F5, D_SHARP5,C_SHARP5,A_SHARP4,0,

  // ♪ 迷う君を
  C_SHARP5,D_SHARP5,F5,E5, D_SHARP5,C_SHARP5,A_SHARP4,0,

  // ♪ 導く光
  A_SHARP4,B4,C_SHARP5,D_SHARP5, C_SHARP5,B4,A_SHARP4,G_SHARP4,

  // ♪ 耳を澄ませば
  C_SHARP5,D_SHARP5,F5,G_SHARP5, F5,D_SHARP5,C_SHARP5,0,

  // ♪ きっと聞こえる
  D_SHARP5,F5,G5,F5, D_SHARP5,C_SHARP5,A_SHARP4,0,

  // ♪ 眠る君に
  C_SHARP5,D_SHARP5,F5,E5, D_SHARP5,C_SHARP5,A_SHARP4,0,

  // ♪ 奏でる歌が
  A_SHARP4,C_SHARP5,D_SHARP5,C_SHARP5,
  A_SHARP4,G_SHARP4,F_SHARP4,0,

  // ラスト
  F_SHARP5
};

// ===== リズム =====
float duration[] = {
  0.5,0.5,1,1, 0.5,0.5,1,1,
  0.5,0.5,1,1, 0.5,0.5,2,2,

  0.5,0.5,1,1, 0.5,0.5,2,2,
  0.5,0.5,1,1, 0.5,1,2,2,

  1,1,1,1, 2,2,

  2,2,

  0.5,0.5,1,1, 0.5,0.5,1,1,
  0.5,0.5,1,1, 0.5,0.5,2,2,

  0.5,0.5,1,1, 0.5,0.5,2,2,
  0.5,0.5,1,1, 0.5,0.5,1,1,

  0.5,0.5,1,1, 0.5,0.5,2,2,
  0.5,0.5,1,1, 0.5,0.5,2,2,

  0.5,0.5,1,1, 0.5,0.5,2,2,

  1,1,1,1,
  1,1,2,2,

  6
};

int length = sizeof(melody) / sizeof(melody[0]);

void setup() {

  randomSeed(analogRead(0));

  for (int i = 0; i < length; i++) {

    int noteDuration = beat * duration[i];

    // テンポ揺らぎ
    float fluctuation = random(92,108)/100.0;
    int realDuration = noteDuration * fluctuation;

    if (melody[i] == 0) {
      delay(realDuration);
    } else {

      int attack = realDuration * 0.2;
      int sustain = realDuration * 0.6;
      int release = realDuration * 0.2;

      tone(buzzerPin, melody[i]);
      delay(attack + sustain);

      tone(buzzerPin, melody[i] - 3);
      delay(release);

      noTone(buzzerPin);
    }

    delay(realDuration * 0.5);
  }

  delay(3000);
}

void loop() {}

*初版: YYYY-MM-DD / AIレビュー: YYYY-MM-DD / グループレビュー後更新: YYYY-MM-DD*
