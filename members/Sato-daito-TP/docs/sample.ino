// ===== ピン定義 =====
const int PIN_BUTTON    = 4;
const int PIN_MODE      = 5;
const int PIN_RESET     = 6;
const int PIN_LED_BLUE  = 8;
const int PIN_LED_GREEN = 9;
const int PIN_LED_RED   = 10;
const int PIN_SOUND_DIGITAL = 3; // デジタル出力
const int PIN_SOUND     = A0;

// ===== 状態 =====
const int STATE_WAIT    = 0;
const int STATE_MEASURE = 1;
const int STATE_NORMAL  = 2;
const int STATE_ALERT   = 3;
const int STATE_WEAK    = 4;

int currentState = STATE_WAIT;

// ===== 変数 =====
int sensorValue = 0;
int maxValue = 0;
int digitalSoundValue = 0;
bool powerState = true;

// ===== タイマー =====
unsigned long lastMillis_LED = 0;
unsigned long lastMillis_Sensor = 0;

// ===== デバウンス =====
unsigned long lastDebounceTime = 0;
const int DEBOUNCE_DELAY = 50;
int lastButtonReading = HIGH;
int stableButtonState = HIGH;

// ===== setup =====
void setup() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_MODE, INPUT_PULLUP);
  pinMode(PIN_RESET, INPUT_PULLUP);

  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

	pinMode(PIN_SOUND_DIGITAL, INPUT);
  pinMode(PIN_SOUND, INPUT);

  Serial.begin(9600);

  currentState = STATE_WAIT;
  sensorValue = 0;
  maxValue = 0;

  // 起動確認
  digitalWrite(PIN_LED_GREEN, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_GREEN, LOW);
}

// ===== loop =====
void loop() {
  unsigned long now = millis();

  readButton();
  readSensor();

  if (!powerState) {
    allOff();
    return;
  }

  switch (currentState) {

    case STATE_WAIT:
      blinkLED(PIN_LED_BLUE, now, 500);

      if (sensorValue > 200) {
        currentState = STATE_MEASURE;
      }
      break;

    case STATE_MEASURE:
  holdMaxValue();

  if (digitalSoundValue == HIGH) {
    currentState = STATE_ALERT;   // 大きい音を即検出
  }
  else if (sensorValue > 600) {
    currentState = STATE_ALERT;
  }
  else if (sensorValue > 200) {
    currentState = STATE_NORMAL;
  }
  else {
    currentState = STATE_WEAK;
  }
  break;
  }

  // デバッグ表示
  Serial.println(sensorValue);

  // リセット
  if (digitalRead(PIN_RESET) == LOW) {
    currentState = STATE_WAIT;
    maxValue = 0;
  }
}

// ===== ボタン =====
bool readButton() {
  // 4番ピン: 電源ON
	if (digitalRead(PIN_BUTTON) == LOW) {
		powerState = true;
		return true;
	}

	// 5番ピン: 電源OFF
	if (digitalRead(PIN_MODE) == LOW) {
		powerState = false;
		return true;
	}

	
  return false;
}

// ===== センサー =====
int readSensor() {
  if (millis() - lastMillis_Sensor >= 100) {
    sensorValue = analogRead(PIN_SOUND);
    
// アナログ値
    digitalSoundValue = digitalRead(PIN_SOUND_DIGITAL); // デジタル値
    lastMillis_Sensor = millis();
  }
  return sensorValue;
}



// ===== 最大値 =====
void holdMaxValue() {
  if (sensorValue > maxValue) {
    maxValue = sensorValue;
  }
}

// ===== 電源 =====
void togglePower() {
  powerState = !powerState;
}

// ===== 全OFF =====
void allOff() {
  digitalWrite(PIN_LED_BLUE, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, LOW);
}

// ===== 点滅 =====
void blinkLED(int pin, unsigned long now, int interval) {
  if (now - lastMillis_LED >= interval) {
    digitalWrite(pin, !digitalRead(pin));
    lastMillis_LED = now;
  }
}