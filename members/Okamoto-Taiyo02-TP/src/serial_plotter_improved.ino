// シリアルプロッタ用改良バージョン
// サウンドセンサーの生値・移動平均・温度・状態を同時にプロット
// Arduino IDEのシリアルプロッタで可視化しやすい形式

#include <DHT.h>
const uint8_t pinSoundSensor = A0;
const uint8_t pinDht11 = 7;
#define DHTTYPE DHT11
DHT dht(pinDht11, DHTTYPE);

int soundBuffer[5] = {0, 0, 0, 0, 0};
int soundValue = 0;
float temperatureC = 0.0;
int soundState = 0; // グローバル変数として定義

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  // サウンドセンサー生値
  int nowSound = analogRead(pinSoundSensor);
  // 移動平均
  for (int i = 4; i > 0; i--) {
    soundBuffer[i] = soundBuffer[i - 1];
  }
  soundBuffer[0] = nowSound;
  int ValueSum = 0;
  for (int i = 0; i < 5; i++) {
    ValueSum += soundBuffer[i];
  }
  soundValue = ValueSum / 5;

  // 温度取得
  temperatureC = dht.readTemperature();

  // 状態判定例（しきい値は適宜調整）
  soundState = (soundValue >= 193) ? 1 : 0;
  Serial.print("Sound Value: ");
  Serial.print(soundValue);
  Serial.print(" | Sound State: ");
  Serial.println(soundState);

  // シリアルプロッタ用出力（カンマ区切り）
  // 生値,移動平均,温度,状態
  Serial.print(nowSound);
  Serial.print(",");
  Serial.print(soundValue);
  Serial.print(",");
  Serial.print(temperatureC);
  Serial.print(",");
  Serial.println(soundState);

  delay(50); // プロット速度調整
}

// Arduino IDEのシリアルプロッタで
// 1列目:サウンド生値 2列目:移動平均 3列目:温度 4列目:検知状態
// を同時に可視化できます。