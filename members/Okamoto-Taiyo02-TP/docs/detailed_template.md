# 詳細設計書 — 組込み開発実習

<!-- 作成者: 岡本太陽 / 日付: 2026-05-25 / グループ: 1-F -->
<!-- ⏱️ 目安時間: 2〜3時間（作成）+ 1時間（AIレビュー・修正） -->

> **このドキュメントの目的**  
> 基本設計書（basic_design.md）で「どのような構造で作るか」を決めました。  
> この詳細設計書では「**各処理を具体的にどう実装するか**」を決めます。  
> 書き終わったとき、**コードの骨格がほぼ完成している**状態を目指してください。

> [!NOTE]
> 詳細設計書を書いた後にコーディングを始めると、書く内容が明確なため格段に速く進みます。  
> 詳細設計書はそのまま **コードのコメント** としても使えます。

---

## 0. 基本設計書との接続確認

> ※ 基本設計書（basic_design.md）の内容を転記して確認します。

| 項目 | 内容（basic_design.md から転記） |
|:--|:--|
| 作品タイトル | 二種類のセンサを用いた送風機 |
| 状態の種類（いくつの状態があるか） | 3 |
| 実装する関数の数 | 15 |

---

## 1. グローバル変数・定数の設計

> ※ プログラム全体で使う変数・定数を**型と初期値まで**決めます。  
> ここで設計した変数は、この後の関数設計でそのまま使います。

```cpp
// =============================================
// ピン定義（基本設計書 2-1 から転記）
// =============================================
const int PIN_BUTTON    =  2;   // タクトスイッチ
const int PIN_LED_RED   =  9;   // 赤LED
const int PIN_LED_GREEN = 10;   // 緑LED
const int PIN_BUZZER    = 11;   // パッシブブザー
// ↓ 自分の部品を追加してください
const uint8_t pinSoundSensor = A0;   // サウンドセンサー（アナログ入力）
const uint8_t pinDht11      = 7;     // DHT11温湿度センサー
const uint8_t pinMotorIn1   = 8;     // モータードライバ入力1（L293D）
const uint8_t pinMotorIn2   = 9;     // モータードライバ入力2（L293D）
const uint8_t pinMotorEn    = 10;    // モータードライバ有効化（L293D、PWM可）
const uint8_t pin7segD1     = 3;     // 7セグメント制御ピン1
const uint8_t pin7segD2     = 4;     // 7セグメント制御ピン2
const uint8_t pin7segD3     = 5;     // 7セグメント制御ピン3
const uint8_t pin7segD4     = 6;     // 7セグメント制御ピン4


// =============================================
// 状態管理（基本設計書 1-2 の状態遷移から）
// =============================================
// 例: 0=待機中, 1=動作中, 2=完了, 3=エラー
int currentState = 0;

// =============================================
// タイマー管理（millis() 用）
// =============================================
unsigned long lastLedTime    = 0;   // LED点滅用タイマー
unsigned long lastSensorTime = 0;   // センサー読み取り用タイマー
// ↓ 自分のタイマーを追加してください
unsigned long soundStartMillis    = 0;   // 音の検知が始まった時刻
unsigned long overTempStartMillis = 0;   // 温度>=28℃が継続し始めた時刻
unsigned long lastSensorReadMillis = 0;  // センサーを最後に読んだ時刻


// =============================================
// センサー・入力値
// =============================================
int sensorValue    = 0;     // センサーの現在値
bool buttonPressed = false; // ボタンの押下状態
// ↓ 自分の変数を追加してください
bool soundDetected   = false;    // サウンドセンサーの検知結果
float temperatureC   = 0.0;      // 現在の温度（℃）
float humidityPct    = 0.0;      // 現在の湿度（%）


// =============================================
// カウンター・フラグ
// =============================================
int score   = 0;     // スコア
bool isActive = false;  // 動作フラグ
// ↓ 自分の変数を追加してください
float motorOnTempC   = 28.0;     // モーターON温度しきい値
float motorOffTempC  = 26.0;     // モーターOFF温度しきい値
unsigned long overTempRequiredMs = 5000; // モーター起動に必要な継続時間（ms）
unsigned long sensorReadIntervalMs = 100; // センサー読取り周期（ms）
bool abnormalLogFlag = false;    // 異常値検出フラグ
char abnormalLogBuffer[64] = ""; // 異常値内容バッファ
```

**↓ 自分のプログラムに合わせて書き直してください**

```cpp
// =============================================
// ピン定義
// =============================================
const uint8_t pinSoundSensor = A0;   // サウンドセンサー（アナログ入力）
const uint8_t pinDht11      = 7;     // DHT11温湿度センサー
const uint8_t pinButton     = 2;     // ボタン（INPUT_PULLUP）
const uint8_t pinMotorIn1   = 8;     // モータードライバ入力1（L293D）
const uint8_t pinMotorIn2   = 9;     // モータードライバ入力2（L293D）
const uint8_t pinMotorEn    = 10;    // モータードライバ有効化（L293D、PWM可）
const uint8_t pinBuzzer     = 12;    // ブザー
const uint8_t pin7segD1     = 3;     // 7セグメント制御ピン1
const uint8_t pin7segD2     = 4;     // 7セグメント制御ピン2
const uint8_t pin7segD3     = 5;     // 7セグメント制御ピン3
const uint8_t pin7segD4     = 6;     // 7セグメント制御ピン4

// --- 確認ポイント ---
// ・ピン番号は配線図・部品表と一致しているか
// ・未使用ピンや競合ピンがないか

// =============================================
// 状態管理
// =============================================
// 0:待機中, 1:モーター作動, 2:機能停止
int currentState = 0;

// =============================================
// タイマー管理
// =============================================
unsigned long soundStartMillis    = 0;   // 音の検知が始まった時刻
unsigned long overTempStartMillis = 0;   // 温度>=28℃が継続し始めた時刻
unsigned long lastSensorReadMillis = 0;  // センサーを最後に読んだ時刻

// =============================================
// センサー・入力値
// =============================================
bool soundDetected   = false;    // サウンドセンサーの検知結果
int soundValue       = 0;        // サウンドセンサーの移動平均値（ノイズ除去後ADC値）
int soundBuffer[5]   = {0, 0, 0, 0, 0}; // サウンドセンサーの過去5回分の値
float temperatureC   = 0.0;      // 現在の温度（℃）
float humidityPct    = 0.0;      // 現在の湿度（%）
bool buttonPressed   = false;    // チャタリング処理後の現在ボタン状態
bool buttonPressEvent = false;   // 押下イベント（非押下→押下の瞬間のみtrue）
// --- チャタリング対策用 ---
bool buttonPrevState = false;           // 前回のボタン状態
unsigned long lastDebounceTime = 0;     // 前回の判定時刻（ms）

// =============================================
// カウンター・フラグ
// =============================================
float motorOnTempC   = 28.0;     // モーターON温度しきい値
float motorOffTempC  = 26.0;     // モーターOFF温度しきい値
unsigned long overTempRequiredMs = 5000; // モーター起動に必要な継続時間（ms）
unsigned long sensorReadIntervalMs = 100; // センサー読取り周期（ms）
bool abnormalLogFlag = false;    // 異常値検出フラグ
char abnormalLogBuffer[64] = ""; // 異常値内容バッファ
```

---

## 2. 各関数の詳細設計

> ※ 基本設計書（3-2 関数一覧）で定義した各関数の「中身」を設計します。  
> **疑似コード**（日本語＋コード混じり）で書いてください。実際のコードは書かなくてOK。

---

### `setup()` — 初期化処理

```
【処理の流れ】
1. ピンモードを設定する
   - PIN_BUTTON → INPUT_PULLUP
   - PIN_LED_RED, PIN_LED_GREEN → OUTPUT
   - PIN_BUZZER → OUTPUT

2. シリアル通信を開始する（デバッグ用）
   - Serial.begin(9600)

3. 必要な場合、ライブラリの初期化を行う
   - （例）lcd.begin(16, 2)
   - （例）servo.attach(PIN_SERVO)

4. 起動音・起動LED（任意）
   - （例）起動時に緑LED を 1 秒点灯してから消灯
```

**↓ 自分の setup() を書いてください**

```
【処理の流れ】
1. ピンモードの設定
   - pinButton → INPUT_PULLUP
   - pinSoundSensor → INPUT
   - pinDht11 → INPUT（DHTライブラリ初期化も実施）
   - pinMotorIn1, pinMotorIn2, pinMotorEn → OUTPUT
   - pinBuzzer → OUTPUT
   - pin7segD1, pin7segD2, pin7segD3, pin7segD4 → OUTPUT

2. シリアル通信を開始する（デバッグ用）
   - Serial.begin(9600)

3. ライブラリ初期化
   - dht.begin() を実行

4. 出力の初期状態を安全側に設定
   - pinMotorIn1 = LOW, pinMotorIn2 = LOW
   - pinMotorEn = 0（モーター停止）
   - pinBuzzer = LOW（ブザー停止）
   - 7セグ表示を消灯（初期表示なし）

5. 状態・タイマー変数の初期化
   - currentState = 0（待機中）
   - soundStartMillis = 0
   - overTempStartMillis = 0
   - lastSensorReadMillis = millis()
```

---

### `loop()` — メインループ

```
【処理の流れ（状態ごとに分岐）】

毎回やること：
- 入力を読む（readInputs()を呼ぶ）
- 現在時刻を取得する（millis()）

currentState が 0（待機中）のとき：
- センサー値を監視する（readSensor()）
- 検知条件を満たしたら → currentState = 1 に変更

currentState が 1（動作中）のとき：
- メイン処理を行う（doMainProcess()）
- 終了条件を満たしたら → currentState = 2 に変更

currentState が 2（完了）のとき：
- 完了表示をする（showResult()）
- リセット入力を待つ → ボタン押したら currentState = 0 に戻す

currentState が 3（エラー）のとき：
- エラー表示をする（showError()）
- リセット入力を待つ
```

**↓ 自分の loop() を書いてください**

```
【処理の流れ】

毎回やること：
   - now = millis() を取得
   - readButton() を呼んで buttonPressEvent を更新（押下イベント方式）
   - now - lastSensorReadMillis >= sensorReadIntervalMs のときのみ
     detectSound(), readTemperature(), readHumidity() を実行して値更新
     lastSensorReadMillis = now に更新
   - judgeSound(), judgeTemperature() を呼んで判定結果を更新
   - abnormalLogFlag が true の場合は logAbnormalValues() を実行

currentState が 0（待機中）のとき：
   - buttonPressEvent が true なら currentState = 2（機能停止）へ遷移
   - soundDetected が true かつ temperatureC >= motorOnTempC のとき：
     - overTempStartMillis が 0 なら overTempStartMillis = now
     - now - overTempStartMillis >= overTempRequiredMs（5000ms）なら
       controlFan() でモーターON（一定速度）
       currentState = 1（モーター作動）へ遷移
   - 上記条件を満たさない場合は overTempStartMillis = 0 に戻す

currentState が 1（モーター作動中）のとき：
    - buttonPressEvent が true なら currentState = 2（機能停止）へ遷移
   - temperatureC <= motorOffTempC（26℃以下）なら
       controlFan() でモーターOFF
       currentState = 0（待機中）へ遷移
   - soundDetected が true の間は soundStartMillis = 0（無検知タイマをリセット）
   - soundDetected が false になった瞬間に soundStartMillis = now を記録
   - soundDetected が false のまま now - soundStartMillis >= 60000 のとき
     currentState = 0（待機中）へ遷移し controlFan() でモーターOFF

currentState が 2（機能停止中）のとき：
   - controlFan() でモーターOFFを維持
   - buttonPressEvent が true（再押下）なら currentState = 0（待機中）へ遷移

異常時（センサー読取り失敗が連続）のとき：
   - 安全側として controlFan() でモーターOFF
   - abnormalLogBuffer に内容を記録し abnormalLogFlag = true

```

---

### （各自の関数を追加してください）

> ※ 基本設計書 3-2 の関数一覧に記載した関数をここに追加していきます。  
> 関数の数だけこのブロックをコピーして使ってください。

---

### `readButton()` — （共通）チャタリング処理済みの押下イベントを返す

**引数：**
- なし

**戻り値：**
- `bool`：押下イベント（非押下→押下になった瞬間のみtrue）

```
【処理の流れ】
1. 「前回のボタン状態」「前回の判定時刻」を使ってデバウンス判定を行う
2. 現在のボタン入力値（digitalRead(pinButton)）を取得する
3. 入力が変化しており、かつ 50ms 以上経過していれば確定入力として buttonPressed を更新する
4. 前回が非押下（HIGH）かつ今回が押下（LOW）のときのみ buttonPressEvent = true にする
5. それ以外は buttonPressEvent = false にする
6. buttonPressEvent を戻り値として返す

【エラーケース】
- digitalRead()が異常値を返した場合やピン未接続時は、安全側（非押下=false）として扱う
```

---

### `detectSound()` — サウンドセンサー値を取得し音検知結果を更新する

**引数：**
- なし

**戻り値：**
- `bool`：音検知結果（検知=true / 未検知=false）

```
【処理の流れ】
1. analogRead(pinSoundSensor) でサウンドセンサーの値を取得し、一時変数に格納
2. 取得値が0～1023の範囲内か isfinite() で確認し、異常値なら前回値を保持し abnormalLogFlag を立てる
3. 5点移動平均を計算し、ノイズを除去した値を soundValue に格納
4. 閾値（例: 40dB相当のADC値）と比較し、音を検知したか判定
5. 判定結果を soundDetected 変数に反映し、true/false を戻り値として返す

【エラーケース】
- analogRead()が範囲外やNaNを返した場合は、前回正常値を保持し、abnormalLogBufferに内容を記録
```

---


### `judgeSound()` — 音の有無と継続時間を判定し状態遷移に反映する

**引数：**
- なし

**戻り値：**
- `void`：なし

```
【処理の流れ】
1. soundDetected が true の場合：soundStartMillis を 0 にリセット（無検知タイマをリセット）
2. soundDetected が false になった瞬間（前回true→今回false）に soundStartMillis = now（millis()）を記録
3. soundDetected が false のまま now - soundStartMillis >= 60000ms（60秒）なら、ファン停止などの状態遷移条件を満たす
4. 状態遷移用のフラグや変数を必要に応じて更新

【エラーケース】
- soundDetected の値が不定や異常な場合は、安全側（無検知扱い）として soundStartMillis を進める
```

---


### `readTemperature()` — DHT11から温度を取得してtemperatureCを更新する

**引数：**
- なし

**戻り値：**
- `float`：取得した温度（℃）

```
【処理の流れ】
1. DHT11ライブラリの readTemperature() で温度値を取得し、一時変数に格納
2. 取得値が isfinite() で有限かつ0～50℃の範囲内か判定。異常値なら前回値を保持し abnormalLogFlag を立てる
3. 正常値なら temperatureC 変数を更新
4. 取得した温度値を戻り値として返す

【エラーケース】
- 取得値がNaNや範囲外の場合は、前回正常値を保持し、abnormalLogBufferに内容を記録
```

---


### `judgeTemperature()` — 温度しきい値を判定して起動・停止条件を決定する

**引数：**
- なし

**戻り値：**
- `void`：なし

```
【処理の流れ】
1. temperatureC >= motorOnTempC（28℃以上）の場合：
   - overTempStartMillis が 0 なら overTempStartMillis = now（millis()）を記録
   - now - overTempStartMillis >= overTempRequiredMs（5000ms）ならファン起動条件を満たす
2. temperatureC < motorOnTempC の場合：overTempStartMillis = 0 にリセット（継続時間をリセット）
3. temperatureC <= motorOffTempC（26℃以下）の場合：ファン停止条件を満たす
4. 状態遷移用のフラグや変数を必要に応じて更新

【エラーケース】
- temperatureC が異常値や不定の場合は、安全側（ファン停止）として扱う
```

---


### `readHumidity()` — DHT11から湿度を取得してhumidityPctを更新する

**引数：**
- なし

**戻り値：**
- `float`：取得した湿度（%）

```
【処理の流れ】
1. DHT11ライブラリの readHumidity() で湿度値を取得し、一時変数に格納
2. 取得値が isfinite() で有限かつ0～100%の範囲内か判定。異常値なら前回値を保持し abnormalLogFlag を立てる
3. 正常値なら humidityPct 変数を更新
4. 取得した湿度値を戻り値として返す

【エラーケース】
- 取得値がNaNや範囲外の場合は、前回正常値を保持し、abnormalLogBufferに内容を記録
```

---


### `controlFan()` — 現在状態に応じてファンのON/OFFを切り替える

**引数：**
- なし

**戻り値：**
- `void`：なし

```
【処理の流れ】
1. currentState が 1（モーター作動）の場合：
   - pinMotorIn1, pinMotorIn2, pinMotorEn をON側（例: In1=HIGH, In2=LOW, En=PWM値）に設定しファンを回す
2. currentState が 0または2（待機/機能停止）の場合：
   - pinMotorIn1, pinMotorIn2, pinMotorEn をOFF側（例: In1=LOW, In2=LOW, En=0）に設定しファンを停止
3. 状態遷移や安全側制御が必要な場合は、常にOFF側を優先

【エラーケース】
- currentStateやピン出力が不定・異常な場合は、安全側（ファンOFF・出力OFF）として全ピンLOW/En=0に設定
```

---


### `handleStop()` — ボタン押下時に機能停止状態への遷移を処理する

**引数：**
- なし

**戻り値：**
- `void`：なし

```
【処理の流れ】
1. ボタン押下時にcurrentState=2（機能停止）へ遷移
2. controlFan()でファンをOFF
3. 必要に応じてブザーや7セグメント表示も停止状態にする

【エラーケース】
- 異常な値が来た場合：
   - currentStateやbuttonPressedが不定の場合も、安全側（ファンOFF・出力OFF）を優先
   - 異常値検出時はabnormalLogBufferに内容を記録し、abnormalLogFlagを立てる
```

---


### `displayTemperature()` — 7セグメントに温度を表示する

**引数：**
- なし

**戻り値：**
- `void`：なし

```
【処理の流れ】
1. temperatureC の値が isfinite() かつ 0～50℃の範囲内か判定する
2. 正常値なら temperatureC を四捨五入し整数化（例：23.4→23）
3. 2桁（十の位・一の位）に分割し、7セグメント表示用のデータに変換
4. 各桁のデータを pin7segD1～D4 に出力（必要に応じて桁消灯やゼロパディング）
5. 小数点は表示しない（整数のみ）
6. 表示後、必要に応じて短い遅延を入れる（ゴースト防止）

【エラーケース】
- temperatureC が isfinite() でない、または 0～50℃の範囲外の場合：
   - 7セグメント全桁を消灯、または "--" 表示に切り替える
   - 異常内容を abnormalLogBuffer に記録し、abnormalLogFlag を true にする
```

---


### `controlAlert()` — ファン作動状態に応じてアラートを制御する

**引数：**
- なし

**戻り値：**
- `void`：なし

```
【処理の流れ】
1. 前回のcurrentStateと現在のcurrentStateを比較し、状態遷移（0→1, 1→0, 1→2, 2→0等）を検出する
2. currentStateが1（モーター作動中）に遷移した瞬間：
   - pinBuzzerを短く1回鳴らす（例：100ms ON→OFF）
3. currentStateが0または2（待機/機能停止）に遷移した瞬間：
   - pinBuzzerを短く2回鳴らす（例：100ms ON→OFFを2回繰り返す）
4. 状態遷移がない場合は、ブザーは鳴らさず、アラート出力は現状維持
5. アラートのON/OFFは状態遷移ごとに必ず明示的に切り替える

【エラーケース】
- currentStateやピン出力が不定・異常な場合は、安全側（アラートOFF）として全アラート出力を停止
- 異常内容をabnormalLogBufferに記録し、abnormalLogFlagをtrueにする
```

---


### `logAbnormalValues()` — 異常値をシリアルにログ出力する

**引数：**
- なし

**戻り値：**
- `void`：なし

```
【処理の流れ】
1. abnormalLogFlag が true かどうか判定
2. true の場合、abnormalLogBuffer の内容を Serial.println() でシリアル出力
3. ログ出力後、abnormalLogFlag を false にリセット
4. abnormalLogBuffer の内容もクリア（空文字にする）

【エラーケース】
- 異常な値が来た場合：シリアル通信が未初期化の場合やバッファ内容が不正な場合は、何もせず安全側（リセットのみ）とする
```

---

## 3. 重要ロジックの設計（必要な場合）

> ※ 複雑なロジック（ゲーム判定、センサー計算、LED点灯パターンなど）は  
> ここでフローチャートや疑似コードで詳しく設計しておきます。

### 3-1. チャタリング防止処理

```
【入力】：
- ボタンの生値（digitalRead(pinButton) の値
- 前回のボタン状態
- 前回の判定時刻

【処理】：
1. 現在のボタン入力値を取得
2. 前回の値と異なる場合、現在時刻との差分を計算
3. 50ms以上経過していれば「状態変化あり」と判定し、ボタン状態を更新
4. 50ms未満ならノイズ（チャタリング）とみなして無視
5. 判定結果をbuttonPressed変数に反映

【出力】：
- チャタリング除去後のボタン状態（true/false）
```

## 疑似コード
```
1. int rawButton = digitalRead(pinButton) で現在のボタン入力値を取得
2. if (rawButton != buttonPrevState) {
      // 状態が変化した場合
      unsigned long now = millis();
      if (now - lastDebounceTime >= 50) {
          // 50ms以上経過していれば有効な変化
          buttonPrevState = rawButton;
          if (rawButton == LOW) {
              buttonPressed = true;  // 押下
          } else {
              buttonPressed = false; // 非押下
          }
      }
      lastDebounceTime = now;
   }
   // 50ms未満なら何もしない（前回の状態を維持）

【出力】：
- buttonPressed（チャタリング除去後のボタン状態 true/false）
```

### 3-2. サウンドセンサー移動平均処理

```
【入力】：
- サウンドセンサーの生値（analogRead(pinSoundSensor) の値）
- 過去4回分のセンサー値

【処理】：
1. 最新値をバッファに追加し、古い値を1つずつずらす
2. バッファ内の5点の平均値を計算
3. 計算結果をsoundValueに格納

【出力】：
- ノイズ除去後のサウンドセンサー値（int型）
```

## 疑似コード
```
1. int nowSoundSensor = analogRead(pinSoundSensor) でセンサーの生値を取得
2. if (nowSoundSensor < 0 || nowSoundSensor > 1023) then
   - abnormalLogBuffer に "Sound sensor out of range" を記録
   - abnormalLogFlag = true
   - 前回の soundValue をそのまま返す
3. グローバル変数 soundBuffer[5] を使い、古い値を後ろへシフトする
   - for i = 4 から 1 まで:
     soundBuffer[i] = soundBuffer[i - 1]
4. soundBuffer[0] = nowSoundSensor を格納
5. sum = soundBuffer[0] + soundBuffer[1] + soundBuffer[2] + soundBuffer[3] + soundBuffer[4]
6. soundValue = sum / 5 を計算して更新
7. 閾値と比較して soundDetected を更新する（例: soundValue >= SOUND_THRESHOLD）
8. soundValue を戻り値として返す
```

### 3-3. 温度しきい値判定ロジック

```
【入力】：
- 現在の温度（temperatureC）
- モーターON/OFFしきい値（motorOnTempC, motorOffTempC）
- 継続時間管理用のタイマー変数

【処理】：
1. temperatureC >= motorOnTempC の場合、overTempStartMillisを記録
2. 一定時間（overTempRequiredMs）継続したらファン起動条件成立
3. temperatureC < motorOnTempC の場合、タイマーをリセット
4. temperatureC <= motorOffTempC の場合、ファン停止条件成立

【出力】：
- ファン起動/停止の判定結果（boolやフラグ）
```

## 疑似コード
```
1. now = millis() を取得
2. if temperatureC が有限値でない または 想定範囲外（例: 0未満 or 50超） then
   - abnormalLogBuffer に "Temperature invalid" を記録
   - abnormalLogFlag = true
   - startConditionMet = false
   - stopConditionMet = true
   - overTempStartMillis = 0
   - return
3. startConditionMet = false
4. stopConditionMet = false
5. if temperatureC >= motorOnTempC then
   - if overTempStartMillis == 0 then
     overTempStartMillis = now
   - if (now - overTempStartMillis) >= overTempRequiredMs then
   startConditionMet = true
6. else
   - overTempStartMillis = 0
7. if temperatureC <= motorOffTempC then
   stopConditionMet = true
8. 判定結果フラグ（startConditionMet / stopConditionMet）を更新して終了
```

### 3-4. 異常値ログ出力処理

```
【入力】：
- abnormalLogFlag
- abnormalLogBuffer

【処理】：
1. abnormalLogFlagがtrueならabnormalLogBufferの内容をSerial出力
2. 出力後、abnormalLogFlagをfalseにリセット
3. abnormalLogBufferを空文字にクリア

【出力】：
- シリアル出力による異常内容の記録
```

## 疑似コード
```
1. if abnormalLogFlag == false then
   - 何もせず終了
2. if abnormalLogBuffer が空文字 then
   - Serial.println("abnormal: unknown")
3. else
   - Serial.print("abnormal: ")
   - Serial.println(abnormalLogBuffer)
4. abnormalLogFlag = false
5. abnormalLogBuffer を空文字にクリア
6. 終了
```

---

## 4. テスト仕様書

> ※ 実装が終わったら、各機能が「正しく動いているか」を確認するテストを定義します。  
> テスト実施後に「実際の結果」と「合否」を記入してください。

### 4-1. 単体テスト（部品・関数ごと）

| No | テスト対象 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | LED（赤）の点灯 | PIN_LED_RED に HIGH を送る | 赤LEDが点灯する | | [ ] |
| 2 | ボタン読み取り | タクトスイッチを押す | buttonPressed が true になる | | [ ] |
| 3 | （自分のテストを追加） | | | | [ ] |
| 4 |  |  |  |  | [ ] |

### 4-2. 機能テスト（必須機能ごと）

> ※ requirements.md の「3-1. 必須機能」を1つずつ検証します。

| No | 必須機能（requirements.md から転記） | テスト手順 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | （requirements.mdの必須機能①を転記） | | | | [ ] |
| 2 | （requirements.mdの必須機能②を転記） | | | | [ ] |
| 3 | （requirements.mdの必須機能③を転記） | | | | [ ] |

### 4-3. 異常系テスト

| No | 異常ケース（basic_design.md 4章から転記） | テスト手順 | 期待する動作 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | センサー値が範囲外 | センサーを遮蔽・最大距離に向ける | 処理がスキップされる | | [ ] |
| 2 | チャタリング | ボタンを素早く2回押す | 1回として認識される | | [ ] |
| 3 | （自分の異常ケースを追加） | | | | [ ] |

---

## 5. AIレビュー記録（詳細設計版）

### Q1: 実装上の問題確認
> 「この詳細設計書に書いた関数と処理フローをもとにArduinoコードを書きます。  
> バグになりやすい箇所・処理の抜け・型の問題はありますか？」

**AIの回答（要約）：**
- 同一loop周回で状態遷移が上書きされる可能性があり、遷移後は `return/continue` または `if-else` で打ち切るべき。
- 判定ロジックが `loop()` 直書きと `judgeSound()/judgeTemperature()` に二重化しており、修正漏れの原因になる。
- `handleStop()/displayTemperature()/controlAlert()` の呼び出し位置が不明確で、実装漏れリスクがある。
- `controlAlert()` は前回状態比較を使うため、`previousState` などの保持変数が必要。
- `detectSound()` の戻り値（bool）と 3-2 疑似コードの戻り値（soundValue）の記述が不一致。
- 3-1 疑似コードが押下イベント方式（`buttonPressEvent`）と一部不整合。
- `now` の取得元（関数内 `millis()` か引数渡し）を統一しないと実装時にぶれやすい。

**対応した内容：**

---

### Q2: テスト仕様の確認
> 「このテスト仕様書で、必須機能がすべて検証できていますか？  
> テストが不足している項目や、境界値テストが必要な箇所を教えてください。」

**AIの回答（要約）：**

**対応した内容：**

---

## 6. グループレビュー記録

### 6-1. 指摘一覧

| No | 指摘内容 | 対応 |
|:---|:---|:---|
| 1 |  |  |
| 2 |  |  |
| 3 |  |  |

### 6-2. レビューを受けて変更した点

- 

---

*初版: 2026-05-22 / グループレビュー後更新: 2026-05-22*
