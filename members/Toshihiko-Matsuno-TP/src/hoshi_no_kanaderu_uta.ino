// 星の奏でる歌 単体再生組込みシステム
// Arduino UNO R3 用
// 必要部品: サウンドセンサ, パッシブブザー, LED, ボタンx4, ファン, 状態表示LEDx3
// 設計: V字モデル・非ブロッキング(millis)・安全停止・チャタリング対策

// --- ピン定義 ---
const int PIN_SOUND = 2;      // サウンドセンサ
const int PIN_BUZZER = 3;     // パッシブブザー
const int PIN_LED = 4;        // メインLED
const int PIN_BUTTON_STOP = 5;  // 強制停止ボタン
const int PIN_FAN = 6;        // ファン
const int PIN_BUTTON_MUSIC = 7;  // 音楽ON/OFF切替
const int PIN_BUTTON_FAN = 8;    // ファンON/OFF切替
const int PIN_BUTTON_LED = 9;    // LED ON/OFF切替
const int PIN_LED_MUSIC_STATUS = 10; // 音楽状態表示LED
const int PIN_LED_FAN_STATUS = 11;   // ファン状態表示LED
const int PIN_LED_LIGHT_STATUS = 12; // LED状態表示LED

// --- 状態定義 ---
enum State {
  WAIT_SOUND,
  PLAY_MUSIC,
  STOP_ALL
};
State state = WAIT_SOUND;

// --- タイマー・フラグ ---
unsigned long musicStartTime = 0;
const unsigned long MUSIC_DURATION = 15000; // 曲の長さ[ms]（例:15秒）
bool isMusicPlaying = false;
bool isFanOn = false;
bool isLedOn = false;
bool enableMusic = true;
bool enableFan = true;
bool enableLight = true;

// --- チャタリング対策 ---
const unsigned long DEBOUNCE_MS = 50;
unsigned long lastStopButtonTime = 0;
unsigned long lastMusicButtonTime = 0;
unsigned long lastFanButtonTime = 0;
unsigned long lastLedButtonTime = 0;

// --- 星の奏でる歌（メロディ配列） ---
// 例: ドレミファソラシド（実際は星の奏でる歌の音階に合わせて修正）
int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};
int noteDurations[] = {4, 4, 4, 4, 4, 4, 4, 4};
const int melodyLength = sizeof(melody) / sizeof(melody[0]);

const int SENSOR_THRESHOLD = 100; // センサモジュールのしきい値設定（例: 100)

void updateStatusLeds() {
  digitalWrite(PIN_LED_MUSIC_STATUS, enableMusic ? HIGH : LOW);
  digitalWrite(PIN_LED_FAN_STATUS, enableFan ? HIGH : LOW);
  digitalWrite(PIN_LED_LIGHT_STATUS, enableLight ? HIGH : LOW);
}

bool readToggleButton(int pin, unsigned long &lastTime) {
  if (digitalRead(pin) == LOW && millis() - lastTime > DEBOUNCE_MS) {
    lastTime = millis();
    return true;
  }
  return false;
}

void handleToggleButtons() {
  if (readToggleButton(PIN_BUTTON_MUSIC, lastMusicButtonTime)) {
    enableMusic = !enableMusic;
    if (!enableMusic) {
      noTone(PIN_BUZZER);
      isMusicPlaying = false;
    } else if (state == PLAY_MUSIC) {
      isMusicPlaying = true;
      playMusic();
    }
  }

  if (readToggleButton(PIN_BUTTON_FAN, lastFanButtonTime)) {
    enableFan = !enableFan;
    if (state == PLAY_MUSIC) {
      controlFan(enableFan);
    } else {
      controlFan(false);
    }
  }

  if (readToggleButton(PIN_BUTTON_LED, lastLedButtonTime)) {
    enableLight = !enableLight;
    if (state == PLAY_MUSIC) {
      controlLed(enableLight);
    } else {
      controlLed(false);
    }
  }

  updateStatusLeds();
}

void setup() {
  pinMode(PIN_SOUND, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON_STOP, INPUT_PULLUP);
  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_BUTTON_MUSIC, INPUT_PULLUP);
  pinMode(PIN_BUTTON_FAN, INPUT_PULLUP);
  pinMode(PIN_BUTTON_LED, INPUT_PULLUP);
  pinMode(PIN_LED_MUSIC_STATUS, OUTPUT);
  pinMode(PIN_LED_FAN_STATUS, OUTPUT);
  pinMode(PIN_LED_LIGHT_STATUS, OUTPUT);

  controlFan(false);
  controlLed(false);
  updateStatusLeds();
}

void loop() {
  handleToggleButtons();

  switch (state) {
    case WAIT_SOUND:
      if (digitalRead(PIN_SOUND) == HIGH) { // センサモジュールのDO端子がHIGHになる
        state = PLAY_MUSIC;
        musicStartTime = millis();
        if (enableMusic) {
          isMusicPlaying = true;
          playMusic();
        }
        controlFan(enableFan);
        controlLed(enableLight);
      }
      break;
    case PLAY_MUSIC:
      if (isMusicPlaying && enableMusic) {
        playMusic();
      } else {
        noTone(PIN_BUZZER);
      }
      if (digitalRead(PIN_BUTTON_STOP) == LOW && millis() - lastStopButtonTime > DEBOUNCE_MS) {
        lastStopButtonTime = millis();
        stopAll();
      }
      if (millis() - musicStartTime > MUSIC_DURATION) {
        stopAll();
      }
      break;
    case STOP_ALL:
      // 停止状態から復帰
      if (digitalRead(PIN_SOUND) == HIGH) {
        state = WAIT_SOUND;
      }
      break;
  }
}

void playMusic() {
  if (!enableMusic) {
    noTone(PIN_BUZZER);
    isMusicPlaying = false;
    return;
  }

  unsigned long elapsed = millis() - musicStartTime;
  int totalDuration = 0;
  for (int i = 0; i < melodyLength; i++) {
    int noteDuration = 1000 / noteDurations[i];
    totalDuration += noteDuration;
    if (elapsed < totalDuration) {
      tone(PIN_BUZZER, melody[i], noteDuration * 0.9);
      break;
    }
  }
  if (elapsed > MUSIC_DURATION) {
    noTone(PIN_BUZZER);
    isMusicPlaying = false;
  }
}

void controlFan(bool on) {
  digitalWrite(PIN_FAN, on ? HIGH : LOW);
  isFanOn = on;
}

void controlLed(bool on) {
  digitalWrite(PIN_LED, on ? HIGH : LOW);
  isLedOn = on;
}

void stopAll() {
  noTone(PIN_BUZZER);
  controlFan(false);
  controlLed(false);
  isMusicPlaying = false;
  state = STOP_ALL;
}
