// ===== ピン定義 =====
const int PIN_BUZZER = 8; 
const int PIN_SENSOR = A0;

// ===== 状態 =====
bool playing = false;

// ===== タイマー =====
unsigned long lastNoteTime = 0;
unsigned long lastTriggerTime = 0;
unsigned long lastClapTime = 0;

// ===== センサー =====
int sensor = 0;
int lastSound = 0;

// ===== 拍手管理 =====
int clapCount = 0;
unsigned long firstClapTime = 0;

// ===== 設定値 =====
const int clapThreshold = 500;
const unsigned long clapWindowMs = 500;
const unsigned long inputBlockMs = 1000;
const unsigned long minClapIntervalMs = 200;

// ===== 曲設定 =====
const int musicCount = 3;
int sound = 0;
int noteIndex = 0;

// 曲の長さ
int notesLength[musicCount] = {8, 8, 8};

// 音データ
int melody[3][8] = {
  {330,330,0,330,0,262,330,392},
  {262,0,196,0,330,0,392,0},
  {523,494,440,392,330,262,196,0}
};

const int noteInterval = 300;

// ===== setup =====
void setup() {
  pinMode(PIN_SENSOR, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  Serial.begin(9600);
  noTone(PIN_BUZZER);

  Serial.println("Start");

  tone(PIN_BUZZER, 440);
  delay(1000);
  noTone(PIN_BUZZER);
}

// ===== loop =====
void loop() {
  unsigned long now = millis();

  sensor = analogRead(PIN_SENSOR);
  sensor = (sensor + lastSound) / 2;
  int diff = abs(sensor - lastSound);

  sensor = analogRead(A0);
  Serial.println(sensor);

  // =========================
  // 1. 異常値チェック（完全無視）
  // =========================
  if (sensor < 10 || sensor > 1000) {
    lastSound = sensor;
    playMusic(now);
    return;
  }

  // =========================
  // 2. 入力無効チェック
  // =========================
  bool blocked = (now - lastTriggerTime) < inputBlockMs;

  if (blocked) {
    // ★重要：無効中はカウントリセット（誤動作防止）
    clapCount = 0;
    firstClapTime = 0;
  }

  // =========================
  // 3. 拍手検出
  // =========================
  bool clapDetected = false;

  if (!blocked) {
    if (diff >= clapThreshold && sensor >= 50) {
      if ((now - lastClapTime) >= minClapIntervalMs) {
        lastClapTime = now;
        clapDetected = true;
      }
    }
  }

  // =========================
  // 4. カウント処理
  // =========================
  if (clapDetected) {
    if (clapCount == 0) {
      firstClapTime = now;
    }
    clapCount++;

    Serial.print("Clap Count: ");
    Serial.println(clapCount);
  }

  // =========================
  // 5. 判定処理
  // =========================
  if (clapCount > 0 && (now - firstClapTime) >= clapWindowMs) {

    if (clapCount == 2) {
      // 再生/停止
      playing = !playing;

      if (!playing) {
        noTone(PIN_BUZZER);
      }

      Serial.println("Toggle Play");
    }
    else if (clapCount == 3) {
      // 曲変更
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

  // =========================
  // 6. 音楽再生
  // =========================
  playMusic(now);

  // =========================
  // 7. 更新
  // =========================
  lastSound = sensor;
}

// ===== 音楽再生 =====
void playMusic(unsigned long now) {
  if (!playing) {
    noTone(PIN_BUZZER);
    return;
  }

  if ((now - lastNoteTime) < noteInterval) {
    return;
  }

  int note = melody[sound][noteIndex];

  if (note > 0) {
    tone(PIN_BUZZER, note);
  } else {
    noTone(PIN_BUZZER);
  }

  noteIndex++;

  if (noteIndex >= notesLength[sound]) {
    noteIndex = 0;
  }

  lastNoteTime = now;
}