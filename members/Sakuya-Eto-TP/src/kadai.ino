// ============================================
// 手で音楽を制御するシステム
// ・2回拍手 → 再生 / 停止
// ・3回拍手 → 曲変更
// ============================================

// ===== ピン定義 =====
const int PIN_BUZZER = 8;     // ブザー（音を出す）
const int PIN_SENSOR = A0;    // サウンドセンサ（音を検出）

// ===== 状態 =====
bool playing = false;         // 音楽再生中かどうか

// ===== タイマー =====
unsigned long lastNoteTime = 0;    // 前の音を鳴らした時刻
unsigned long lastTriggerTime = 0; // 最後に操作した時刻
unsigned long lastClapTime = 0;    // 最後の拍手検出時刻

// ===== センサー =====
int sensor = 0;        // 現在の音の大きさ
int lastSound = 0;     // 1つ前の音の大きさ

// ===== 拍手管理 =====
int clapCount = 0;               // 拍手回数
unsigned long firstClapTime = 0; // 最初の拍手時刻

// ===== 設定値 =====
const int clapThreshold = 10;            // 拍手と判断する差分
const unsigned long clapWindowMs = 1000;  // 拍手回数を判定する時間
const unsigned long inputBlockMs = 1000;  // 操作後の無効時間
const unsigned long minClapIntervalMs = 200; // 連続検出防止

// ===== 曲設定 =====
const int musicCount = 3;   // 曲の数
int sound = 0;              // 現在の曲番号
int noteIndex = 0;          // 現在の音の位置

// ===== 曲の長さ =====
int notesLength[musicCount] = {30, 22, 44};

// ===== 曲データ =====
int melody[3][50] = {

  // ======================================
  // 🎵 曲1：かっこいい・ゆっくり（30音）
  // ======================================
  {523,659,784,659,523,659,784,880,
 784,659,523,659,784,988,1046,988,
 784,659,523,659,784,880,784,659,
 523,659,784,1046,880,0,

 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

  // ======================================
  // 🎵 曲2：暗い（22音）
  // ======================================
  {247,262,247,233,220,196,220,185,
 220,233,247,220,196,185,175,165,
 175,185,196,175,165,0,
 
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

  // ======================================
  // 🎵 曲3：速い・怖い（44音）
  // ======================================
  {220,247,262,247,220,196,220,185,
   220,247,262,247,220,196,185,175,

   175,196,175,165,175,196,220,247,
   262,247,220,196,175,165,175,196,

   220,196,175,165,147,165,175,196,
   220,262,220,196,175,165,147,0}
};

// ===== 音の切り替え間隔 =====
int noteInterval[musicCount] = {180, 140, 70};

// ============================================
// setup：初期化処理（1回だけ）
// ============================================
void setup() {
  pinMode(PIN_SENSOR, INPUT);   // センサを入力に設定
  pinMode(PIN_BUZZER, OUTPUT);  // ブザーを出力に設定

  Serial.begin(9600);           // シリアル通信開始
  Serial.println("Start");

  // 起動確認音
  tone(PIN_BUZZER, 440);
  delay(500);
  noTone(PIN_BUZZER);
}

// ============================================
// loop：メイン処理（繰り返し）
// ============================================
void loop() {
  unsigned long now = millis();  // 現在時間取得

  // ===== センサ読み取り =====
  sensor = analogRead(PIN_SENSOR);

  // ノイズ除去（前回との平均）
  sensor = (sensor + lastSound) / 2;

  // 変化量（拍手検出に使用）
  int diff = abs(sensor - lastSound);

  Serial.println(sensor);

  // ===== 異常値チェック =====
  if (sensor < 10 || sensor > 1000) {
    lastSound = sensor;
    playMusic(now);
    return;
  }

  // ===== 入力無効チェック =====
  bool blocked = (now - lastTriggerTime) < inputBlockMs;

  if (blocked) {
    clapCount = 0;
    firstClapTime = 0;
  }

  // ===== 拍手検出 =====
  bool clapDetected = false;

  if (!blocked) {
    // 音の変化が大きければ拍手と判断
    if (diff >= clapThreshold) {
      if ((now - lastClapTime) >= minClapIntervalMs) {
        lastClapTime = now;
        clapDetected = true;
      }
    }
  }

  // ===== 拍手カウント =====
  if (clapDetected) {
    if (clapCount == 0) firstClapTime = now;
    clapCount++;
 
   Serial.print("Clap Count: ");
   Serial.println(clapCount);
  }

  // ===== 判定処理 =====
  if (clapCount > 0 && (now - firstClapTime) >= clapWindowMs) {

    // 2回 → 再生/停止
    if (clapCount == 2) {
      playing = !playing;

      if (!playing) {
        noTone(PIN_BUZZER);
      }

      Serial.println("Toggle Play");
    }

    // 3回 → 曲変更
    else if (clapCount == 3) {
      sound = (sound + 1) % musicCount;
      noteIndex = 0;
      playing = true;

      Serial.println("Change Music");
    }

    // リセット
    clapCount = 0;
    firstClapTime = 0;
    lastTriggerTime = now;
  }

  // ===== 音楽再生 =====
  playMusic(now);

  // 次回の比較用に保存
  lastSound = sensor;
}

// ============================================
// 音楽再生関数
// ============================================
void playMusic(unsigned long now) {

  // 再生していなければ停止
  if (!playing) {
    noTone(PIN_BUZZER);
    return;
  }

  // 音の切り替えタイミング制御
  if ((now - lastNoteTime) < noteInterval[sound]) {
    return;
  }

  // 現在の音を取得
  int note = melody[sound][noteIndex];

  // 音がある場合は鳴らす
  if (note > 0) {
    tone(PIN_BUZZER, note);
  } else {
    noTone(PIN_BUZZER);
  }
  // 次の音へ
  noteIndex++;

  // 曲の最後なら最初に戻る
  if (noteIndex >= notesLength[sound]) {
    noteIndex = 0;
  }

  // 時刻更新
  lastNoteTime = now;
}