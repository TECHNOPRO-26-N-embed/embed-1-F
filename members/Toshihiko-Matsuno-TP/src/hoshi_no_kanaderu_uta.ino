// 星の奏でる歌 単体再生組込みシステム
// Arduino UNO R3 用（ファン機能は仕様から除外）
// 必要部品: サウンドセンサ, パッシブブザー, メインLED, ボタンx3, 状態表示LEDx2

// --- ピン定義 ---
const int PIN_SOUND = A0;               // サウンドセンサ AO
const int PIN_BUZZER = 3;               // パッシブブザー
const int PIN_LED_MAIN = 4;             // メインLED
const int PIN_BUTTON_STOP = 5;          // 強制停止ボタン
const int PIN_BUTTON_MUSIC = 7;         // 音楽ON/OFF切替ボタン
const int PIN_BUTTON_LED = 9;           // LED ON/OFF切替ボタン
const int PIN_LED_MUSIC_STATUS = 10;    // 音楽有効状態LED
const int PIN_LED_LIGHT_STATUS = 12;    // LED有効状態LED

// --- しきい値設定 ---
const int SOUND_THRESHOLD_ON = 100;      // 音量がこの値以上でトリガー
const int SOUND_THRESHOLD_OFF = 95;      // 復帰用ヒステリシス

// --- 状態定義 ---
enum State {
  WAIT_SOUND,
  PLAY_MUSIC,
  STOP_ALL
};

State state = WAIT_SOUND;

// --- 各種フラグ ---
bool enableMusic = true;
bool enableLight = true;
bool isMusicPlaying = false;
bool isMainLedOn = false;

// --- チャタリング対策 ---
const unsigned long DEBOUNCE_MS = 50;
unsigned long lastStopButtonTime = 0;
unsigned long lastMusicButtonTime = 0;
unsigned long lastLedButtonTime = 0;

// --- 再トリガー制御 ---
unsigned long stopEnteredAt = 0;
const unsigned long STOP_HOLD_MS = 300;

// --- 星の奏でる歌（詳細設計 8-1 準拠） ---
const int TEMPO = 75;
const float BEAT_MS = 60000.0 / TEMPO;

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

int melody[] = {
  A_SHARP4, A_SHARP4, B4, C_SHARP5, C_SHARP5, B4, A_SHARP4, G_SHARP4,
  A_SHARP4, B4, C_SHARP5, D_SHARP5, C_SHARP5, B4, A_SHARP4, 0,

  C_SHARP5, D_SHARP5, F_SHARP5, E5, D_SHARP5, C_SHARP5, A_SHARP4, 0,
  A_SHARP4, B4, C_SHARP5, D_SHARP5, C_SHARP5, A_SHARP4, G_SHARP4, 0,

  A_SHARP4, C_SHARP5, D_SHARP5, C_SHARP5, A_SHARP4, 0,
  0, 0,

  D_SHARP5, C_SHARP5, B4, A_SHARP4, G_SHARP4, A_SHARP4, B4, C_SHARP5,
  D_SHARP5, F5, G5, F5, D_SHARP5, C_SHARP5, A_SHARP4, 0,

  C_SHARP5, D_SHARP5, F5, E5, D_SHARP5, C_SHARP5, A_SHARP4, 0,
  A_SHARP4, B4, C_SHARP5, D_SHARP5, C_SHARP5, B4, A_SHARP4, G_SHARP4,

  C_SHARP5, D_SHARP5, F5, G_SHARP5, F5, D_SHARP5, C_SHARP5, 0,
  D_SHARP5, F5, G5, F5, D_SHARP5, C_SHARP5, A_SHARP4, 0,

  C_SHARP5, D_SHARP5, F5, E5, D_SHARP5, C_SHARP5, A_SHARP4, 0,
  A_SHARP4, C_SHARP5, D_SHARP5, C_SHARP5, A_SHARP4, G_SHARP4, F_SHARP4, 0,

  F_SHARP5
};

float duration[] = {
  0.5, 0.5, 1, 1, 0.5, 0.5, 1, 1,
  0.5, 0.5, 1, 1, 0.5, 0.5, 2, 2,

  0.5, 0.5, 1, 1, 0.5, 0.5, 2, 2,
  0.5, 0.5, 1, 1, 0.5, 1, 2, 2,

  1, 1, 1, 1, 2, 2,
  2, 2,

  0.5, 0.5, 1, 1, 0.5, 0.5, 1, 1,
  0.5, 0.5, 1, 1, 0.5, 0.5, 2, 2,

  0.5, 0.5, 1, 1, 0.5, 0.5, 2, 2,
  0.5, 0.5, 1, 1, 0.5, 0.5, 1, 1,

  0.5, 0.5, 1, 1, 0.5, 0.5, 2, 2,
  0.5, 0.5, 1, 1, 0.5, 0.5, 2, 2,

  0.5, 0.5, 1, 1, 0.5, 0.5, 2, 2,
  1, 1, 1, 1, 1, 1, 2, 2,

  6
};

const int melodyLength = sizeof(melody) / sizeof(melody[0]);

// --- 再生制御 ---
int currentNoteIndex = 0;
unsigned long noteStartAt = 0;
bool noteToneOn = false;

void updateStatusLeds() {
  digitalWrite(PIN_LED_MUSIC_STATUS, enableMusic ? HIGH : LOW);
  digitalWrite(PIN_LED_LIGHT_STATUS, enableLight ? HIGH : LOW);
}

bool readToggleButton(int pin, unsigned long &lastTime) {
  if (digitalRead(pin) == LOW && millis() - lastTime > DEBOUNCE_MS) {
    lastTime = millis();
    return true;
  }
  return false;
}

void controlMainLed(bool on) {
  digitalWrite(PIN_LED_MAIN, on ? HIGH : LOW);
  isMainLedOn = on;
}

void startMusic() {
  isMusicPlaying = true;
  currentNoteIndex = 0;
  noteStartAt = millis();
  noteToneOn = false;
}

void stopOutputs() {
  noTone(PIN_BUZZER);
  controlMainLed(false);
  isMusicPlaying = false;
}

void stopAll() {
  stopOutputs();
  state = STOP_ALL;
  stopEnteredAt = millis();
}

void handleToggleButtons() {
  if (readToggleButton(PIN_BUTTON_MUSIC, lastMusicButtonTime)) {
    enableMusic = !enableMusic;
    if (!enableMusic && state == PLAY_MUSIC) {
      stopAll();
    }
  }

  if (readToggleButton(PIN_BUTTON_LED, lastLedButtonTime)) {
    enableLight = !enableLight;
    if (!enableLight) {
      controlMainLed(false);
    }
  }

  updateStatusLeds();
}

void updateMusic() {
  if (!isMusicPlaying || !enableMusic) {
    return;
  }

  if (currentNoteIndex >= melodyLength) {
    stopAll();
    return;
  }

  unsigned long noteDuration = (unsigned long)(BEAT_MS * duration[currentNoteIndex]);
  unsigned long toneDuration = (noteDuration * 9UL) / 10UL;
  int noteFreq = melody[currentNoteIndex];
  unsigned long elapsed = millis() - noteStartAt;

  // 音を鳴らす区間だけLED点灯し、拍の隙間は消灯
  if (!noteToneOn) {
    if (noteFreq > 0) {
      tone(PIN_BUZZER, noteFreq);
      if (enableLight) {
        controlMainLed(true);
      }
    } else {
      noTone(PIN_BUZZER);
      controlMainLed(false);
    }
    noteToneOn = true;
  }

  if (noteToneOn && elapsed >= toneDuration) {
    noTone(PIN_BUZZER);
    controlMainLed(false);
    noteToneOn = false;
  }

  if (elapsed >= noteDuration) {
    currentNoteIndex++;
    noteStartAt = millis();
    noteToneOn = false;
  }
}

void setup() {
  pinMode(PIN_SOUND, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_MAIN, OUTPUT);
  pinMode(PIN_BUTTON_STOP, INPUT_PULLUP);
  pinMode(PIN_BUTTON_MUSIC, INPUT_PULLUP);
  pinMode(PIN_BUTTON_LED, INPUT_PULLUP);
  pinMode(PIN_LED_MUSIC_STATUS, OUTPUT);
  pinMode(PIN_LED_LIGHT_STATUS, OUTPUT);

  stopOutputs();
  updateStatusLeds();
}

void loop() {
  handleToggleButtons();

  // 強制停止は全状態で最優先
  if (readToggleButton(PIN_BUTTON_STOP, lastStopButtonTime)) {
    stopAll();
  }

  int soundValue = analogRead(PIN_SOUND);

  switch (state) {
    case WAIT_SOUND:
      if (soundValue >= SOUND_THRESHOLD_ON) {
        if (enableMusic) {
          startMusic();
          state = PLAY_MUSIC;
        } else {
          // 音楽無効時は誤連続トリガー防止のため一度停止状態へ
          stopAll();
        }
      }
      break;

    case PLAY_MUSIC:
      updateMusic();
      break;

    case STOP_ALL:
      // 最低保持時間を過ぎ、音が十分下がったら待機へ戻す
      if (millis() - stopEnteredAt >= STOP_HOLD_MS && soundValue <= SOUND_THRESHOLD_OFF) {
        state = WAIT_SOUND;
      }
      break;
  }
}
