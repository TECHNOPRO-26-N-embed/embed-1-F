# 詳細設計書 — 組込み開発実習

<!-- 作成者: 衛藤咲哉 / 日付: 2026-05-25 / グループ: 1-F -->

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
| 作品タイトル | 拍手が鳴ったら音楽の再生、停止  |
| 状態の種類（1-2 状態遷移から） | [電源ON / 初期化] ↓（初期化完了）<br>[待機中] ──（センサー検知）─→ [計測中] ──（閾値超え）─→(2回) [音楽再生/停止] ─（センサー検知）<br>└→(3回) [次の曲を再生]<br>└→（閾値未満）─→ [無視] |
| 実装する関数の数（2-2 関数一覧から） | 14　個 |
| グローバル変数の合計バイト数（2-1 SRAM確認から） | 23　B |

---

## 1. グローバル変数・定数の設計

> ※ 基本設計書（2-1 データ設計）をもとに、**型と初期値まで**決めます。
> ここで設計した変数は、この後の関数設計でそのまま使います。

```
【ピン定義】（basic_design.md 3-1 から転記）
  PIN_BUZZER    = 8   // パッシブブザー
  PIN_SENSOR    = A0   // サウンドセンサ


【状態管理】（basic_design.md 1-2 の状態名から転記）
  playing : bool = false              // 再生中かどうか(falseがオフ)

【タイマー（millis()用）】（basic_design.md 2-3 から転記）
  unsigned long lastMillis_ClapJudge = 0;  // 0.5秒判定
  unsigned long lastNoteTime = 0;          // 前回音から、次の音へ進む基準時刻
  unsigned long lastTriggerTime = 0;       // 入力無効

【センサー・入力値】（basic_design.md 2-1 から転記）
  sensor           : int            = 0   // サウンドセンサの現在の計測値
  lastSound        : int            = 0   // 1つ前のセンサ値（変化量で拍手検出）
  clapCount        : int            = 0   // 一定時間内に検出した拍手回数
  firstClapTime    : unsigned long  = 0   // 最初の拍手を検知した時刻（millis）


【その他のフラグ・カウンター】
  clapThreshold   : const int            = 100   // 拍手判定のしきい値（変化量）
  clapWindowMs    : const unsigned long  = 500   // 拍手回数を確定する時間
  inputBlockMs    : const unsigned long  = 1000  // 再生/停止直後の入力無効時間
  musicCount      : const int            = 3     // 用意する曲数(仮)

  lastClapTime    : unsigned long        = 0;    // 最小間隔（チャタリング防止）
  minClapIntervalMs: const unsigned long = 50; // この時間未満の再検出は無視
　　
【出力制御】
  sound         : int            = 0  // 曲番号
  noteIndex     : int            = 0  // 現在の音
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
1. ピンモードを設定する
   - PIN_SENSOR → INPUT（サウンドセンサ入力）
   - PIN_BUZZER → OUTPUT（ブザー出力）

2. Serial.begin(9600)（デバッグ用）
   - シリアル通信を開始する（動作確認に使用）

3. 初期状態の設定
   - playing を false（音楽停止）に設定する
   - sound を 0（最初の曲）に設定する
   - noteIndex を 0（最初の音）に設定する

4. タイマー変数の初期化
   - 拍手判定の時刻（lastMillis_ClapJudge）を 0 にする
   - 前回音を鳴らした時刻（lastNoteTime）を 0 にする
   - 最後に操作を受け付けた時刻（lastTriggerTime）を 0 にする
   - 最後に拍手を検出した時刻（lastClapTime）を 0 にする

5. センサー・入力値の初期化
   - 現在のセンサー値（sensor）を 0 にする
   - 1つ前のセンサー値（lastSound）を 0 にする
   - 拍手回数（clapCount）を 0 にする
   - 最初の拍手時刻（firstClapTime）を 0 にする

6. 出力の初期状態
   - noTone(PIN_BUZZER) を実行し、ブザーを停止する

7. 起動確認（任意）
   - シリアルモニタに「Start」などを表示する
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

1. 現在時刻とセンサー値を取得する
  - now = millis() を取得する
  - sensor = analogRead(PIN_SENSOR) で現在値を読む
  - diff = abs(sensor - lastSound) を計算する

2. 入力無効時間中かどうかを確認する
  - (now - lastTriggerTime) < inputBlockMs の場合:
    ・clapCount = 0 にリセットする
    ・firstClapTime = 0 にリセットする
    ・lastSound = sensor を更新する
    ・このループでは拍手関連処理（手順3〜5）をスキップし、手順6へ進む


3. 拍手を検出する（多重検出防止付き）
  - diff >= clapThreshold を満たすか判定する
  - かつ (now - lastClapTime) >= minClapIntervalMs の場合のみ拍手として採用する
  - 条件を満たした場合:
      ・lastClapTime = now に更新
      ・拍手検出成功として手順4へ進む
  - 条件を満たさない場合:
      ・clapCount は変更せず、そのまま手順6へ進む

4. 拍手回数をカウントする
  - clapCount == 0 のとき firstClapTime = now を記録する
  - clapCount を +1 する

5. 拍手ウィンドウ終了後に回数を確定する
  - clapCount > 0 かつ (now - firstClapTime) >= clapWindowMs の場合:

    - clapCount == 2 のとき:
        ・playing を反転（再生／停止）
        ・停止時は noTone(PIN_BUZZER) を実行する

    - clapCount == 3 のとき:
        ・sound = (sound + 1) % musicCount
        ・noteIndex = 0 に戻す
        ・playing = true にする

    - 判定後:
        ・lastTriggerTime = now に更新
        ・clapCount = 0 にリセット
        ・firstClapTime = 0 にリセット

6. 音楽再生処理を実行する
  - playing == true のときのみ再生処理を実行する
  - (now - lastNoteTime) で音の切り替えタイミングを判定する
  - 切り替えタイミングになった場合:
      ・現在の音を再生する（tone）
      ・noteIndex を +1 する
      ・曲データの末尾に到達した場合は noteIndex を 0 に戻す
      ・lastNoteTime = now に更新する
  - playing == false のとき:
      ・noTone(PIN_BUZZER) を維持する

7. 次ループ用に前回値を保存する
  - lastSound = sensor を保存する
```

```

---

### （関数ごとに以下のブロックをコピーして追加してください）

> ※ 基本設計書 2-2 の関数一覧に記載した関数を1つずつ設計します。

---

### `関数名()` — （役割を1行で書く）
setup():
  ピン設定・変数初期化・シリアル開始・ブザー停止を行う。

loop():
  センサ取得・拍手判定・音楽再生・状態更新を毎ループ実行する。

readSensor():
  サウンドセンサの値を読み取り、sensorに保存して返す。

isInputBlocked(now):
  入力無効時間中かどうかを判定し、結果を返す。

updateClap(diff, now):
  しきい値と最小間隔を満たす場合のみ拍手として検出し、カウントを更新する。

processClap(now):
  一定時間後に拍手回数を確定し、再生・停止・曲変更を実行する。

togglePlaying():
  再生状態を反転し、停止時はブザーの音を止める。

changeMusic():
  曲番号を次に進め、再生位置を先頭に戻す。

playMusic(now):
  再生中の場合のみ、時間に応じて音を更新しブザーで再生する。
```

**basic_design.md 2-2 との対応：** （基本設計書の関数一覧の説明を転記）
| 機能ID | 機能名 | 関数名 | 担う「1つの仕事」 | 主な引数 | 戻り値 | 呼び出す場所 |
|:--|:--|:--|:--|:--|:--|:--|
| — | 初期化 | `setup()` | ピンモード設定・ライブラリ初期化・起動確認 | なし | なし | 起動時1回 |
| — | （共通）待機・制御 | `loop()` | 状態に応じて各関数を呼び出すメインループ | なし | なし | 常時 |
| — | （共通）ボタン読出 | `readButton()` | チャタリング処理済みのボタン状態を返す | なし | bool | loop()内 |
| — | （共通）センサー読出 | `readSensor()` | センサー値を取得して sensorValue を更新 | なし | int (cm) | loop()内 |
| — | （共通）出力更新 | `updateOutput()` | 現在の state に応じて LED/ブザーを制御 | int state | なし | loop()内 |
| F1 | センサ読み取り | readSensor | サウンドセンサの値を取得してsensorに格納 | なし | int | loop |
| F2 | 拍手検出 | detectClap | sensorとlastSoundの差から拍手を検出 | sensor | bool | loop |
| F3 | 拍手カウント管理 | updateClap | clapCountとfirstClapTimeを更新 | 検出結果 | なし | loop |
| F4 | 拍手判定 | processClap | 2回・3回判定して処理を決定 | なし | なし | loop |
| F5 | 再生状態切替 | togglePlaying | playingのON/OFF切替 | なし | なし | F4内 |
| F6 | 曲変更 | changeMusic | sound（曲番号）を変更しnoteIndexをリセット | なし | なし | F4内 |
| F7 | 音楽再生 | playMusic | playingがtrueなら音を再生 | なし | なし | loop |
| F8 | 音更新 | updateNote | noteIndexとlastNoteTimeで次の音へ進む | なし | なし | F7内 |
| F9 | 入力無効判定 |isInputBlocked | lastTriggerTimeから1秒以内か確認 | なし | bool | loop |

**引数：** `引数名`（型）: 何の値か

setup():
　なし

loop():
　なし

readSensor():
　なし

isInputBlocked(now):
　now（unsigned long）: 現在時刻（millisの値）

updateClap(diff, now):
　diff（int）: センサ値の変化量
　now（unsigned long）: 現在時刻（millisの値）

processClap(now):
　now（unsigned long）: 現在時刻（millisの値）

togglePlaying():
　なし

changeMusic():
　なし

playMusic(now):
　now（unsigned long）: 現在時刻（millisの値）

**戻り値：** 型（なしの場合は void）
setup():
　戻り値：void

loop():
　戻り値：void

readSensor():
　戻り値：int（センサ値）

isInputBlocked(now):
　戻り値：bool（入力無効ならtrue）

updateClap(diff, now):
　戻り値：bool（拍手が検出されたらtrue）

processClap(now):
　戻り値：void

togglePlaying():
　戻り値：void

changeMusic():
　戻り値：void

playMusic(now):
　戻り値：void

【処理の流れ】
- 初期化処理とメインループで記載済み

【エラー・異常ケース】
- 異常な値が来た場合:
  - センサ値が正常範囲外（例：10未満または1000以上）の場合は異常とみなす
  - 異常値の場合は拍手検出およびカウント処理を行わない
  - lastSound = sensor を更新して次のループに備える

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス処理）

> ※ ボタンを使う場合は必ず設計してください。

```
【考え方】
  拍手されたとき、50ms 以内の連続入力は無視する。

【チャタリング防止処理】
1. サウンドセンサの変化量を確認する
　・diff >= clapThreshold を満たすか判定する
2. 前回の拍手検出からの時間を確認する
　・(now - lastClapTime) >= minClapIntervalMs を満たすか判定する
3. 両方の条件を満たしている場合のみ拍手として採用する
　・clapCount を +1 する
　・lastClapTime = now に更新する
4. 条件を満たさない場合は無視する
　・拍手回数は増やさない
　・何も処理しない


【必要な変数（Section 1 に追加済みか確認）】
  lastClapTime        : 最後に拍手を検出した時刻
  minClapIntervalMs   : 最小間隔（例：50ms）
```

---

### 3-2. millis() を使ったタイマー管理

```
【考え方】
  「現在時刻 now」と「前回時刻（基準時刻）」の差分で時間を判定し、
  delay() を使わずに複数処理（拍手判定・入力無効・再生）を並行で制御する。

【処理の流れ（例: LED点滅）】
1. 現在時刻を取得する
  ・now = millis() により現在の経過時間を取得する
2. 経過時間を計算する
  ・elapsed = now - 基準時刻 を求める
3. しきい時間を超えたか判定する
  ・elapsed >= 設定時間（例: 500ms）で条件成立とする
4. 条件成立時のみ処理を実行する
  ・対象処理を実行し、基準時刻を更新する
5. 条件不成立時は何もしない
  ・次ループで再判定する

【自分のシステムで millis() を使う処理】
1. 拍手判定ウィンドウ（firstClapTime）
  ・条件: clapCount > 0 かつ (now - firstClapTime) >= clapWindowMs
  ・役割: 一定時間内の拍手回数（2回/3回）を確定する

2. 入力無効時間（lastTriggerTime）
  ・条件: (now - lastTriggerTime) < inputBlockMs
  ・役割: 再生/停止や曲変更の直後に誤検出を受け付けない

3. 音楽再生タイミング（lastNoteTime）
  ・条件: playing == true かつ (now - lastNoteTime) >= 音符の長さ
  ・役割: 次の音へ進め、lastNoteTime を更新する

4. チャタリング防止（lastClapTime）
  ・条件: (now - lastClapTime) >= minClapIntervalMs
  ・役割: 近接した連続入力を同一拍手として無視する
```
---

### 3-3. その他の重要ロジック（任意）

> **【任意】** 複雑なロジックがある場合のみ記入してください。
> 例：「距離に応じたLED点灯パターン」「ゲームの衝突判定」「温度の閾値判定」

```
【処理の流れ】
1. 拍手確定条件を判定する
  - clapCount > 0 かつ (now - firstClapTime) >= clapWindowMs を満たすか確認する
  - 条件未成立なら何もしないで次ループへ進む

2. 拍手回数に応じて動作を分岐する
  - clapCount == 2 の場合: togglePlaying() を実行する（再生/停止を切替）
  - clapCount == 3 の場合: changeMusic() を実行し、playing = true にする
  - それ以外（1回、4回以上）の場合: 操作としては無効にして状態を変更しない

3. 判定後の共通後処理を行う
  - lastTriggerTime = now に更新する（入力無効時間の起点）
  - clapCount = 0 にリセットする
  - firstClapTime = 0 にリセットする

【入力値と出力値の関係】
  - 入力: clapCount, firstClapTime, now, playing, sound, noteIndex
  - 出力:
    ・2回拍手: playing が反転（true/false切替）
    ・3回拍手: sound が次の曲番号へ進み、noteIndex = 0、playing = true
    ・その他: playing/sound/noteIndex は変更なし
  - 共通更新: 判定実施後は lastTriggerTime を更新し、clapCount と firstClapTime を初期化

```

---

## 4. デバッグ出力計画（任意）

> **【任意】** 関数設計（Section 2）と並行して記入すると効果的です。
> 「動かない」ときに何を確認すればいいかを事前に計画しておきます。
> 実装後は不要な Serial.println() を削除すること。

| No | 確認したい内容 | 挿入する関数 | Serial.println の内容例 |
|:--|:--|:--|:--|
| 1 | センサ値が正しく取得できているか | readSensor | String("sensor=") + sensor |
| 2 | センサ変化量（diff）が正しく計算されているか | loop | String("diff=") + diff |
| 3 | 拍手が検出されたか | updateClap | "Clap detected" |
| 4 | 拍手回数が正しくカウントされているか | updateClap | String("clapCount=") + clapCount |
| 5 | 入力無効状態かどうか | isInputBlocked | "Input Blocked" |
| 6 | 拍手回数の判定結果 | processClap | String("clapCount judged=") + clapCount |
| 7 | 再生/停止が切り替わったか | togglePlaying | String("playing=") + (playing ? "ON" : "OFF") |
| 8 | 曲が切り替わったか | changeMusic | String("music changed, sound=") + sound |
| 9 | 音の再生タイミングが正常か | playMusic | String("noteIndex=") + noteIndex |
|10 | 音が再生されているか | playMusic | "Playing sound" |
|11 | 音が停止状態か | playMusic | "Stopped" |
|12 | 入力無効時間の経過確認 | loop | String("BlockTime=") + (now - lastTriggerTime) |

---

## 5. 単体テスト仕様書（V字モデル：詳細設計 ↔ 単体テスト）

> ※ 各関数・部品が「単体で正しく動くか」を確認するテスト項目を設計します。
> 「実際の結果」欄は実装後に記入します。

### 5-1. 入力系テスト

| No   | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:--|:--|:--|:--|:--|:--|
| UT-I01 | readSensor | 無音状態でセンサを読む | sensorが低い値（10未満など）になる |  |  |
| UT-I02 | readSensor | 強い音（拍手）を与える | sensorが高い値を示す（しきい値以上） |  |  |
| UT-I03 | readSensor | センサ値が連続して変化する状況 | sensorが連続的に変化する |  |  |
| UT-I04 | isInputBlocked | now - lastTriggerTime < inputBlockMs の状態 | true（入力無効）が返る |  |  |
| UT-I05 | isInputBlocked | now - lastTriggerTime >= inputBlockMs の状態 | false（入力可能）が返る |  |  |
| UT-I06 | updateClap | diff < clapThreshold の入力 | 拍手として検出されない（clapCount変化なし） |  |  |
| UT-I07 | updateClap | diff >= clapThreshold の入力 | 拍手として検出され（clapCount+1） |  |  |
| UT-I08 | updateClap | 50ms以内に2回入力する | 2回目は無視される（clapCount増えない） |  |  |
| UT-I09 | updateClap | 50ms以上間隔を空けて入力 | 拍手として2回カウントされる |  |  |
| UT-I10 | updateClap | 異常値（0や1023など極端な値） | 拍手として扱われない |  |  |
| UT-I11 | loop（入力無効時） | 入力無効時間中に拍手を行う | 拍手がカウントされない |  |  |
| UT-I12 | loop（前回値更新） | 入力無効中にセンサ変化がある | lastSound が更新される |  |  |
| UT-I13 | updateClap | diff == clapThreshold の入力 | 拍手として検出される（clapCount+1） |  |  |
| UT-I14 | updateClap | ちょうど50ms間隔で2回入力する | 2回目が有効としてカウントされる |  |  |
| UT-I15 | isInputBlocked | now - lastTriggerTime == inputBlockMs の状態 | false（入力可能）が返る |  |  |

### 5-2. 出力系テスト

| No   | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:--|:--|:--|:--|:--|:--|
| UT-O01 | togglePlaying | playing=false の状態で関数実行 | playing=true になる（再生開始） |  |  |
| UT-O02 | togglePlaying | playing=true の状態で関数実行 | playing=false になる（停止） |  |  |
| UT-O03 | togglePlaying | 停止切替時 | noTone(PIN_BUZZER) が実行される |  |  |
| UT-O04 | changeMusic | sound=0 の状態で関数実行 | sound=1 になる |  |  |
| UT-O05 | changeMusic | sound=最大値（musicCount-1）で実行 | sound=0 に戻る（循環） |  |  |
| UT-O06 | changeMusic | 関数実行時 | noteIndex が 0 にリセットされる |  |  |
| UT-O07 | playMusic | playing=true ＆ 時間経過前 | 音は更新されない（noteIndex変化なし） |  |  |
| UT-O08 | playMusic | playing=true ＆ (now - lastNoteTime) >= 間隔 | tone() が実行される |  |  |
| UT-O09 | playMusic | 音切替時 | noteIndex が +1 される |  |  |
| UT-O10 | playMusic | 曲末尾到達時 | noteIndex が 0 に戻る |  |  |
| UT-O11 | playMusic | playing=false の状態で実行 | noTone(PIN_BUZZER) が維持される |  |  |
| UT-O12 | playMusic | 停止中に複数ループ実行 | 音が一切鳴らない状態が続く |  |  |
| UT-O13 | playMusic | playing=true ＆ (now - lastNoteTime) == 間隔 | ちょうど境界で音更新が実行される |  |  |
| UT-O14 | changeMusic | 連続で2回関数実行 | sound が2曲分進み、範囲内で循環する |  |  |
| UT-O15 | togglePlaying | 再生中→停止へ切替直後に playMusic 実行 | 音が再開せず停止状態を維持する |  |  |


### 5-3. タイミング・並行動作テスト

| No   | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:--|:--|:--|:--|:--|:--|
| UT-T01 | loop（拍手判定） | 0.5秒以内に2回拍手 | 2回と認識され再生/停止が切替される |  |  |
| UT-T02 | loop（拍手判定） | 0.5秒以上間隔で2回拍手 | 別々の1回として扱われ動作しない |  |  |
| UT-T03 | loop（拍手判定） | 0.5秒以内に3回拍手 | 曲が変更される |  |  |
| UT-T04 | loop（入力無効） | 操作直後に拍手を行う | 入力が無効となり動作しない |  |  |
| UT-T05 | loop（入力無効解除） | 1秒経過後に拍手 | 再び入力が有効になり動作する |  |  |
| UT-T06 | playMusic | 再生中に拍手（2回）を行う | 音楽再生中でも停止に切り替わる |  |  |
| UT-T07 | playMusic | 再生中に3回拍手 | 再生を止めずに曲が変更される |  |  |
| UT-T08 | playMusic | 再生中に何も入力しない | 一定間隔で音が順番に再生され続ける |  |  |
| UT-T09 | playMusic | 音再生中に次のタイミング到達 | noteIndex が進み連続再生される |  |  |
| UT-T10 | loop（並行動作） | 音再生中にセンサ入力を与える | 音再生と拍手検出が同時に動作する |  |  |
| UT-T11 | loop（処理遅延確認） | 連続して音と拍手を与える | 拍手検出と音再生が遅延なく同時に処理される |  |  |
| UT-T12 | loop（チャタリング＋時間） | 50ms以内に連続入力 | 1回の拍手として処理される |  |  |
| UT-T13 | loop（境界値時間） | ちょうど0.5秒で入力 | 正しく判定される（2回または無効が仕様通り） |  |  |
| UT-T14 | loop（長時間動作） | 長時間連続動作させる | 誤動作なく安定して再生・検出される |  |  |
| UT-T15 | loop（入力無効境界） | now - lastTriggerTime == inputBlockMs で拍手 | 入力有効として判定処理が再開される |  |  |
| UT-T16 | loop（無効解除直後） | 入力無効終了直後の最初の拍手 | 1回目の拍手として正しくカウントされる |  |  |
| UT-T17 | loop（無効中の3回拍手） | 入力無効時間中に3回拍手 | 曲変更されず、clapCount がリセットされる |  |  |
---

## 6. AIレビュー記録

> グループレビューの前に必ず実施してください。

### Q1: 実装上の問題確認

> 「この詳細設計書に書いた関数と処理フローをもとに Arduino でコードを書きます。バグになりやすい箇所・処理の抜け・型の問題はありますか？」

**AIの回答（要約）：**入力無効時間前後で判定が不自然に途切れる

**対応した内容：**

---

### Q2: 単体テスト仕様の確認

> 「Section 5 の単体テスト仕様書で、各関数の動作が正しく検証できていますか？テストが不足している項目や、境界値テストが必要な箇所を教えてください。」

**AIの回答（要約）：**

**対応した内容：**

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

*初版: YYYY-MM-DD / AIレビュー: YYYY-MM-DD / グループレビュー後更新: YYYY-MM-DD*
