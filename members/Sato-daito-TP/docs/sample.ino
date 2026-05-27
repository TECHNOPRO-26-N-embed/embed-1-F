
// ===== ピン定義 =====
// 入力系: ボタン、モード切替、リセット
const int PIN_BUTTON    = 4;
const int PIN_MODE      = 5;
const int PIN_RESET     = 6;
// 出力系: LED(青/緑/赤)
const int PIN_LED_BLUE  = 8;
const int PIN_LED_GREEN = 9;
const int PIN_LED_RED   = 10;
// アナログ入力: 音センサー
const int PIN_SOUND     = A0;

// ===== 状態 =====
// 待機中(青点滅)
const int STATE_WAIT    = 0;
// 測定中(最大値保持と状態判定)
const int STATE_MEASURE = 1;
// 正常(緑点灯)
const int STATE_NORMAL  = 2;
// 警告(赤点滅)
const int STATE_ALERT   = 3;
// 弱い信号(青点灯)
const int STATE_WEAK    = 4;

int currentState = STATE_WAIT;

// ===== 変数 =====
int sensorValue = 0;
int maxValue = 0;
// 電源状態(ボタンでON/OFF切替)
bool powerState = true;

// ===== タイマー =====
// LED点滅制御用タイムスタンプ
unsigned long lastMillis_LED = 0;
// センサー読み取り間隔制御用タイムスタンプ
unsigned long lastMillis_Sensor = 0;

// ===== デバウンス =====
// チャタリング対策: 最後に入力が変化した時刻
unsigned long lastDebounceTime = 0;
const int DEBOUNCE_DELAY = 50;
// 直前の生入力値
int lastButtonReading = HIGH;
// 安定したボタン状態
int stableButtonState = HIGH;

// ===== setup =====
void setup() {
	// 入力ピン(内部プルアップ有効)
	pinMode(PIN_BUTTON, INPUT_PULLUP);
	pinMode(PIN_MODE, INPUT_PULLUP);
	pinMode(PIN_RESET, INPUT_PULLUP);

	// 出力ピン
	pinMode(PIN_LED_BLUE, OUTPUT);
	pinMode(PIN_LED_GREEN, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);

	// 音センサー入力
	pinMode(PIN_SOUND, INPUT);

	// シリアル通信開始(デバッグ表示用)
	Serial.begin(9600);

	// 初期状態
	currentState = STATE_WAIT;
	sensorValue = 0;
	maxValue = 0;

	// 起動確認: 緑LEDを1秒点灯
	digitalWrite(PIN_LED_GREEN, HIGH);
	delay(1000);
	digitalWrite(PIN_LED_GREEN, LOW);
// Arduino標準関数・マクロ利用のため必須
#include <Arduino.h>

// ===== loop =====
void loop() {
	unsigned long now = millis();

	// 入力更新
	readButton();
	readSensor();

	// 電源OFF中は全LED消灯して処理を抜ける
	if (!powerState) {
		allOff();
		return;
	}

	// 状態ごとの処理
	switch (currentState) {

		case STATE_WAIT:
			// 待機中は青LEDをゆっくり点滅
			blinkLED(PIN_LED_BLUE, now, 500);

			// 閾値を超えたら測定へ遷移
			if (sensorValue > 200) {
				currentState = STATE_MEASURE;
			}
			break;

		case STATE_MEASURE:
			// 測定中は最大値を記録
			holdMaxValue();

			// センサー値で状態を分類
			if (sensorValue > 600) currentState = STATE_ALERT;
			else if (sensorValue > 200) currentState = STATE_NORMAL;
			else currentState = STATE_WEAK;

			break;

		case STATE_WEAK:
			// 弱い信号: 青点灯
			digitalWrite(PIN_LED_BLUE, HIGH);
			break;

		case STATE_NORMAL:
			// 正常: 緑点灯
			digitalWrite(PIN_LED_GREEN, HIGH);
			break;

		case STATE_ALERT:
			// 警告: 赤点滅を速めに
			blinkLED(PIN_LED_RED, now, 200);
			break;
	}

	// デバッグ表示(現在のセンサー値)
	Serial.println(sensorValue);

	// リセットボタン押下で状態と最大値を初期化
	if (digitalRead(PIN_RESET) == LOW) {
		currentState = STATE_WAIT;
		maxValue = 0;
	}
}

// ===== ボタン =====
bool readButton() {
	int reading = digitalRead(PIN_BUTTON);

	// 入力変化があればデバウンスタイマーを更新
	if (reading != lastButtonReading) {
		lastDebounceTime = millis();
	}

	// 一定時間安定した入力のみ確定状態として採用
	if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
		if (reading != stableButtonState) {
			stableButtonState = reading;

			// 押下検出(LOW)で電源状態をトグル
			if (stableButtonState == LOW) {
				togglePower();
				return true;
			}
		}
	}

	lastButtonReading = reading;
	return false;
}

// ===== センサー =====
int readSensor() {
	// 100msごとに音センサーをサンプリング
	if (millis() - lastMillis_Sensor >= 100) {
		sensorValue = analogRead(PIN_SOUND);
		lastMillis_Sensor = millis();
	}
	return sensorValue;
}

// ===== 最大値 =====
void holdMaxValue() {
	// 観測した最大センサー値を保持
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
	// 指定間隔ごとにLEDを反転して点滅させる
	if (now - lastMillis_LED >= interval) {
		digitalWrite(pin, !digitalRead(pin));
		lastMillis_LED = now;
	}
}
