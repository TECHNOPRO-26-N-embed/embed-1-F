// --- ピン ---
const int PIN_SOUND = A0;
const int PIN_BUZZER = 3;
const int PIN_LED_MAIN = 4;
const int PIN_BUTTON_STOP = 5;
const int PIN_BUTTON_MUSIC = 7;
const int PIN_BUTTON_LED = 9;
const int PIN_LED_MUSIC_STATUS = 10;
const int PIN_LED_LIGHT_STATUS = 12;

// --- 状態 ---
enum State { WAIT_SOUND, PLAY_MUSIC, STOP_ALL };
State state = WAIT_SOUND;

// --- テンポ ---
float beat1 = 60000.0 / 75;
float beat2 = 60000.0 / 140;

// --- 再生管理 ---
int currentNoteIndex = 0;
unsigned long noteStartAt = 0;
bool noteToneOn = false;

// --- フラグ ---
bool enableLight = true;
bool isPlaying = false;

// --- ボタン ---
const unsigned long DEBOUNCE = 50;
unsigned long lastMusicBtn = 0;
unsigned long lastStopBtn = 0;
unsigned long lastLedBtn = 0;

// --- 曲選択 ---
int currentSong = 0;

// =========================
// 音定義
// =========================
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

#define E4 330
#define G4 392
#define A4 440
#define C5 523
#define D5 587

// =========================
// 曲1（完全）
// =========================
int melody1[] = {
A_SHARP4,A_SHARP4,B4,C_SHARP5,C_SHARP5,B4,A_SHARP4,G_SHARP4,
A_SHARP4,B4,C_SHARP5,D_SHARP5,C_SHARP5,B4,A_SHARP4,0,
C_SHARP5,D_SHARP5,F_SHARP5,E5,D_SHARP5,C_SHARP5,A_SHARP4,0,
A_SHARP4,B4,C_SHARP5,D_SHARP5,C_SHARP5,A_SHARP4,G_SHARP4,0,
A_SHARP4,C_SHARP5,D_SHARP5,C_SHARP5,A_SHARP4,0,
0,0,
D_SHARP5,C_SHARP5,B4,A_SHARP4,G_SHARP4,A_SHARP4,B4,C_SHARP5,
D_SHARP5,F5,G5,F5,D_SHARP5,C_SHARP5,A_SHARP4,0,
C_SHARP5,D_SHARP5,F5,E5,D_SHARP5,C_SHARP5,A_SHARP4,0,
A_SHARP4,B4,C_SHARP5,D_SHARP5,C_SHARP5,B4,A_SHARP4,G_SHARP4,
C_SHARP5,D_SHARP5,F5,G_SHARP5,F5,D_SHARP5,C_SHARP5,0,
D_SHARP5,F5,G5,F5,D_SHARP5,C_SHARP5,A_SHARP4,0,
C_SHARP5,D_SHARP5,F5,E5,D_SHARP5,C_SHARP5,A_SHARP4,0,
A_SHARP4,C_SHARP5,D_SHARP5,C_SHARP5,A_SHARP4,G_SHARP4,F_SHARP4,0,
F_SHARP5
};

float dur1[] = {
0.5,0.5,1,1,0.5,0.5,1,1,
0.5,0.5,1,1,0.5,0.5,2,2,
0.5,0.5,1,1,0.5,0.5,2,2,
0.5,0.5,1,1,0.5,1,2,2,
1,1,1,1,2,2,
2,2,
0.5,0.5,1,1,0.5,0.5,1,1,
0.5,0.5,1,1,0.5,0.5,2,2,
0.5,0.5,1,1,0.5,0.5,2,2,
0.5,0.5,1,1,0.5,0.5,1,1,
0.5,0.5,1,1,0.5,0.5,2,2,
0.5,0.5,1,1,0.5,0.5,2,2,
0.5,0.5,1,1,0.5,0.5,2,2,
1,1,1,1,1,1,2,2,
6
};

// =========================
// 曲2（完全）
// =========================
int melody2[] = {
E4,G4,A4,B4,A4,G4,E4,
G4,A4,B4,C5,B4,A4,G4,
A4,B4,C5,D5,C5,B4,A4,
B4,C5,B4,G4,E4,
0,0,
E4,G4,A4,B4,A4,G4,E4,
G4,A4,B4,C5,B4,A4,G4,
A4,B4,C5,D5,C5,B4,A4,
B4,C5,B4,G4,E4,
0,
E5
};

float dur2[] = {
0.5,0.5,1,1,1,1,2,
0.5,0.5,1,1,1,1,2,
0.5,0.5,1,1,1,1,2,
1,1,2,2,2,
2,2,
0.5,0.5,1,1,1,1,2,
0.5,0.5,1,1,1,1,2,
0.5,0.5,1,1,1,1,2,
1,1,2,2,2,
2,
6
};

// --- ボタン ---
bool btn(int pin, unsigned long &t) {
  if (digitalRead(pin)==LOW && millis()-t>DEBOUNCE) {
    t=millis(); return true;
  }
  return false;
}

// --- LED表示 ---
void updateSongLed() {
  digitalWrite(PIN_LED_MUSIC_STATUS, (currentSong == 0) ? HIGH : LOW);
}

void updateStatusLeds() {
  digitalWrite(PIN_LED_LIGHT_STATUS, enableLight ? HIGH : LOW);
}

// --- 再生制御 ---
void startMusic() {
  isPlaying = true;
  currentNoteIndex = 0;
  noteStartAt = millis();
  noteToneOn = false;
}

void stopAll() {
  noTone(PIN_BUZZER);
  digitalWrite(PIN_LED_MAIN, LOW);
  isPlaying = false;
  state = STOP_ALL;
}

void updateMusic() {

  if (!isPlaying) return;

  int *mel;
  float *dur;
  float beat;
  int len;

  if (currentSong==0) {
    mel=melody1; dur=dur1; beat=beat1;
    len=sizeof(melody1)/sizeof(int);
  } else {
    mel=melody2; dur=dur2; beat=beat2;
    len=sizeof(melody2)/sizeof(int);
  }

  if (currentNoteIndex>=len) {
    stopAll(); return;
  }

  unsigned long noteDur = beat * dur[currentNoteIndex];
  unsigned long elapsed = millis()-noteStartAt;
  int freq = mel[currentNoteIndex];

  if (!noteToneOn) {
    if (freq>0) {
      tone(PIN_BUZZER,freq);
      if (enableLight) digitalWrite(PIN_LED_MAIN,HIGH);
    }
    noteToneOn=true;
  }

  if (noteToneOn && elapsed > noteDur*0.8) {
    if (freq>0) tone(PIN_BUZZER,freq-3);
  }

  if (elapsed >= noteDur) {
    noTone(PIN_BUZZER);
    digitalWrite(PIN_LED_MAIN,LOW);
    currentNoteIndex++;
    noteStartAt = millis();
    noteToneOn=false;
  }
}

// --- setup ---
void setup() {
  pinMode(PIN_SOUND,INPUT);
  pinMode(PIN_BUZZER,OUTPUT);
  pinMode(PIN_LED_MAIN,OUTPUT);
  pinMode(PIN_LED_MUSIC_STATUS,OUTPUT);
  pinMode(PIN_LED_LIGHT_STATUS,OUTPUT);
  pinMode(PIN_BUTTON_STOP,INPUT_PULLUP);
  pinMode(PIN_BUTTON_MUSIC,INPUT_PULLUP);
  pinMode(PIN_BUTTON_LED,INPUT_PULLUP);
}

// --- loop ---
void loop() {

  updateSongLed();
  updateStatusLeds();

  if (btn(PIN_BUTTON_MUSIC,lastMusicBtn)) {
    currentSong = (currentSong+1)%2;
  }

  if (btn(PIN_BUTTON_LED,lastLedBtn)) {
    enableLight = !enableLight;
  }

  if (btn(PIN_BUTTON_STOP,lastStopBtn)) {
    stopAll();
  }

  int sound = analogRead(PIN_SOUND);

  switch(state) {

    case WAIT_SOUND:
      if (sound>100) {
        startMusic();
        state=PLAY_MUSIC;
      }
      break;

    case PLAY_MUSIC:
      updateMusic();
      break;

    case STOP_ALL:
      if (sound<95) state=WAIT_SOUND;
      break;
  }
}
